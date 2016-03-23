//
//  LDPRTViewController.m
//  ldprt_ios
//
//  Created by iaccepted on 16/3/14.
//  Copyright (c) 2016年 iaccepted. All rights reserved.
//

#import "LDPRTViewController.h"
#import "Scene.h"
#import "Renderer.h"
#import "Sampler.h"
#import "Light.h"
#import "Global.h"

@interface LDPRTViewController ()
{
    Renderer renderer;
    Scene scene;
    Sampler sampler;
    Light light;
    Directions directions;
    CGFloat prex;
    CGFloat prey;
    CGFloat theta;
    CGFloat phi;
}

@property (strong, nonatomic) EAGLContext *context;

@end

@implementation LDPRTViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
}

//set up GL
- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    [self initialWork];
}

//做一些渲染前的准备工作：模型加载，sample生成，renderer初始化
-(void)initialWork
{
    glEnable(GL_DEPTH_TEST);
    
    prex = 0.0;
    prey = 0.0;
    theta = 167.1f;
    phi = 186.2f;
    
    sampler.generateSamples();
    
    const char *pathOne = [self getPath:@"012" :@"obj"];
    scene.addModelFromFile(pathOne);
    
    directions.generateDir(SQRT_DIR_NUM);
    
    bool ret = scene.generateCoeffsAndLobes(sampler, directions);
    if (!ret) {
        exit(-2);
    }
    
    light.lightFromImage("beach_probe.pfm", sampler);
    light.rotateLightCoeffs(theta, phi);
    
    renderer.compileShaderFromFile("verShader.glsl", "fragShader.glsl");
    renderer.setUniform();
}
//开启渲染流程
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    renderer.renderSceneWithLight(scene, light);
}

//some small functions
- (const char *)getPath:(NSString *)fileName :(NSString *)type
{
    NSString *path = [[NSBundle mainBundle] pathForResource:fileName ofType:type];
    const char *pathString = [path UTF8String];
    return pathString;
}

@end
