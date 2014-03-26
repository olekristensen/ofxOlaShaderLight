//
//  ofxOlaShaderLight.cpp
//  Trae
//
//  Created by Ole Kristensen on 22/03/14.
//
//

#include "ofxOlaShaderLight.h"

bool ofxOlaShaderLight::shaderSetup = false;
ofxUboShader * ofxOlaShaderLight::shader = new ofxUboShader();
ofxOlaShaderLight::Light ofxOlaShaderLight::lightStruct = ofxOlaShaderLight::Light();

vector<DMXfixture*> * DMXfixture::DMXfixtures = new vector<DMXfixture*>;
bool DMXfixture::oladSetup = false;

#ifdef USE_OLA_LIB_AND_NOT_OSC
 ola::DmxBuffer * DMXfixture::buffer = new ola::DmxBuffer();
 ola::StreamingClient * DMXfixture::ola_client = new ola::StreamingClient();
#else
ofxOscSender * DMXfixture::oscSender = new ofxOscSender();
int * DMXfixture::buffer = new int[512];
#endif // LINUX/*


