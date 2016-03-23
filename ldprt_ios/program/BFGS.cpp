#include "BFGS.h"

LFLOAT *BFGS::tlm = new LFLOAT[BAND_NUM * BAND_NUM];
SAMPLE *BFGS::samples = new SAMPLE[SQRT_SAMPLES_NUM * SQRT_SAMPLES_NUM];
int BFGS::n = 0;
lbfgsfloatval_t *BFGS::x = NULL;
lbfgs_parameter_t BFGS::param;

void BFGS::init()
{
	n = 3 + BAND_NUM * BAND_NUM;
	x = lbfgs_malloc(n);

	lbfgs_parameter_init(&param);
}

LFLOAT gradA(LFLOAT x, LFLOAT y, LFLOAT z)
{
	LFLOAT a = y * y + z * z;
	LFLOAT b = a + x * x;
	b = sqrt(b);
	b = b * b * b;
	return a / b;
}

LFLOAT gradB(LFLOAT x, LFLOAT y, LFLOAT z)
{
	LFLOAT a = -y * z;
	LFLOAT b = x * x + y * y + z * z;
	b = sqrt(b);
	b = b * b * b;
	return a / b;
}

static int progress(
	void *instance,
	const lbfgsfloatval_t *x,
	const lbfgsfloatval_t *g,
	const lbfgsfloatval_t fx,
	const lbfgsfloatval_t xnorm,
	const lbfgsfloatval_t gnorm,
	const lbfgsfloatval_t step,
	int n,
	int k,
	int ls
	)
{
	printf("Iteration %d:\n", k);
	printf("  fx = %f\n", fx);
	//printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
	printf("\n");
	return 0;
}

lbfgsfloatval_t BFGS::evaluate(
	void *instance,
	const lbfgsfloatval_t *x,
	lbfgsfloatval_t *g,
	const int n,
	const lbfgsfloatval_t step
	)
{
	int nFuncs = BAND_NUM * BAND_NUM;
	LFLOAT approximateTlm[BAND_NUM * BAND_NUM];
	SHEvalFunc SHEval[] = { SHEval3, SHEval3, SHEval3, SHEval3, SHEval4, SHEval5, SHEval6, SHEval7, SHEval8, SHEval9, SHEval10 };
	
	LFLOAT lx = x[0], ly = x[1], lz = x[2];
	LFLOAT D = sqrt(lx * lx + ly * ly + lz * lz);
	lx /= D;
	ly /= D;
	lz /= D;

	LFLOAT *shs = new LFLOAT[nFuncs];
	
	SHEval[BAND_NUM](lx, ly, lz, shs);
	
	for (int i = 0; i < nFuncs; ++i)
	{
		approximateTlm[i] = x[i + 3] * shs[i];
	}

	//calculate the fx
	lbfgsfloatval_t fx = 0;	

	int nSamples = SQRT_SAMPLES_NUM * SQRT_SAMPLES_NUM;
	for (int sampleIdx = 0; sampleIdx < nSamples; ++sampleIdx)
	{
		LFLOAT ta = 0.0f, tb = 0.0f;
		for (int l = 0; l < nFuncs; ++l)
		{
			ta += samples[sampleIdx].shValues[l] * tlm[l]; 
			tb += samples[sampleIdx].shValues[l] * approximateTlm[l];
		}
		fx += (ta - tb) * (ta - tb);
	}

	//calculate the gradient

	//1.approximateTlm - tlm -> approximateTlm��
	for (int l = 0; l < nFuncs; ++l)
	{
		approximateTlm[l] -= tlm[l];
	}

	//2.calculate gradient of gl*
	int index = 3;
	for (int l = 0; l < BAND_NUM; ++l)
	{
		
		LFLOAT gradGl = 0;
		for (int m = -l; m <= l; ++m)
		{
			gradGl += shs[INDEX(l, m)] * approximateTlm[INDEX(l, m)];
		}
		gradGl *= 2.0f;
		for (int j = 0; j < 2 * l + 1; ++j)
		{
			g[index + j] = gradGl;
		}
		index += 2 * l + 1;
	}

	//3.calculate gradient of gradient of ylm (ie the sh coefficients) ����s*��Ӧ����гϵ�����ݶ�
	LFLOAT gradYlmTheta[BAND_NUM * BAND_NUM]; // record the gradient of theta
	LFLOAT gradYlmPhi[BAND_NUM * BAND_NUM];

	LFLOAT theta = acos(lz);
	LFLOAT phi = acos(lx / (sqrt(lx * lx + ly * ly)));

	//calculate the gradient of x, y, z about theta and phi
	LFLOAT gradThetaX = 0, gradThetaY = 0, gradThetaZ = (-1.0f / (sqrt(1 - lz * lz)));
	LFLOAT gradPhiX = fabs(ly) / (lx * lx + ly * ly);
	LFLOAT gradPhiY = -1.0 * lx * lx / (fabs(ly));
	LFLOAT gradPhiZ = 0;

	LFLOAT gap = 0.001;
	for (int l = 0; l < BAND_NUM; ++l)
	{
		for (int m = -l; m <= l; ++m)
		{
			double val = SH(l, m, theta, phi);
			double theta1 = SH(l, m, theta + gap, phi);

			double phi1 = SH(l, m, theta, phi + gap);


			/*gradYlmTheta[INDEX(l, m)] = gradTheta(l, m, theta, phi);
			gradYlmPhi[INDEX(l, m)] = gradPhi(l, m, theta, phi);*/

			gradYlmTheta[INDEX(l, m)] = (theta1 - val) / gap;
			gradYlmPhi[INDEX(l, m)] = (phi1 - val) / gap;

			/*cout << "������"  << gradYlmTheta[INDEX(l, m)] << "   " << gradYlmPhi[INDEX(l, m)] << endl;
			cout << "��ֵ��" << (ytheta1 - y) / gap << "   " << (yphi1 - y) / gap << endl;*/
		}
	}	



	//the gradient of x, y, z about the objective function


	LFLOAT lxx = gradA(x[0], x[1], x[2]);
	LFLOAT lxy = gradB(x[2], x[0], x[1]);
	LFLOAT lxz = gradB(x[1], x[0], x[2]);
	LFLOAT lyx = gradB(x[2], x[0], x[1]);
	LFLOAT lyy = gradA(x[1], x[0], x[2]);
	LFLOAT lyz = gradB(x[0], x[1], x[2]);
	LFLOAT lzx = gradB(x[1], x[0], x[2]);
	LFLOAT lzy = gradB(x[0], x[1], x[2]);
	LFLOAT lzz = gradB(x[2], x[0], x[1]);

	LFLOAT gradXObj = 0.0, gradYObj = 0.0, gradZObj = 0.0;
	index = 3;

	for (int l = 0; l < BAND_NUM; ++l)
	{
		for (int m = -l; m <= l; ++m)
		{
			int loc = INDEX(l, m);
			LFLOAT gradYlmX = gradYlmTheta[loc] * gradThetaX + gradYlmPhi[loc] * gradPhiX;
			LFLOAT gradYlmY = gradYlmTheta[loc] * gradThetaY + gradYlmPhi[loc] * gradPhiY;
			LFLOAT gradYlmZ = gradYlmTheta[loc] * gradThetaZ + gradYlmPhi[loc] * gradPhiZ;

			//У�����ڽ�x,y,z���б�׼������ݶ�Ӱ��
			gradYlmX = gradYlmX * lxx + gradYlmY * lyx + gradYlmZ * lzx;
			gradYlmY = gradYlmX * lxy + gradYlmY * lyy + gradYlmZ * lzy;
			gradYlmZ = gradYlmX * lxz + gradYlmY * lyz + gradYlmZ * lzz;

			gradXObj += gradYlmX * x[index + loc] * approximateTlm[loc];
			gradYObj += gradYlmY * x[index + loc] * approximateTlm[loc];
			gradZObj += gradYlmZ * x[index + loc] * approximateTlm[loc];

		}
	}

	gradXObj *= 2.0;
	gradYObj *= 2.0;
	gradZObj *= 2.0;

	//���ݶ���Ϣд��g
	g[0] = gradXObj;
	g[1] = gradYObj;
	g[2] = gradZObj;

	return fx;
}

void BFGS::bfgs(lbfgsfloatval_t *fx)
{
	//N, x, &fx, evaluate, progress, NULL, &param
	//lbfgs(n, x, fx, evaluate, progress, NULL, &param);
	lbfgs(n, x, fx, evaluate, NULL, NULL, &param);
}