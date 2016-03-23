/***************************************
*author: guohongzhi zju
*date:2015.8.22
*func: ��2D�ռ������Ȼ����ӳ�䵽������
*Ȼ�����ÿ���������SH
****************************************/
#ifndef _SAMPLER_H_
#define _SAMPLER_H_

#include "Global.h"


class SAMPLE
{
public:
	glm::vec3 xyz;
	LFLOAT theta;
	LFLOAT phi;
	LFLOAT shValues[BAND_NUM * BAND_NUM];

	SAMPLE(const SAMPLE& sample);
	SAMPLE(){}
	~SAMPLE()
	{
	}
};

class Sampler
{
public:
	std::vector<SAMPLE> samples;

	//get the size of samples
	unsigned size() const;
	//generate samples
	void generateSamples();
	SAMPLE &operator[] (unsigned int index);
};
#endif