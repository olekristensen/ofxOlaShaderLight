//
//  ofxOlaShaderLight.h
//  Trae
//
//  Created by Ole Kristensen on 22/03/14.
//
//

#pragma once

#include "ofMain.h"
#ifdef USE_OLA_LIB_AND_NOT_OSC
#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/StreamingClient.h>
#else
#include "ofxOsc.h"
#endif
#include "ofxUbo.h"

#define MAX_SHADER_LIGHTS 512

class DMXchannel
{
public:
    enum DMXchannelType
    {
        DMX_CHANNEL_RED,
        DMX_CHANNEL_GREEN,
        DMX_CHANNEL_BLUE,
        DMX_CHANNEL_WHITE,
        DMX_CHANNEL_CW,
        DMX_CHANNEL_WW,
        DMX_CHANNEL_COLOR_TEMPERATURE,
        DMX_CHANNEL_BRIGHTNESS,
        DMX_CHANNEL_HUE,
        DMX_CHANNEL_SATURATION
    };

    DMXchannel(unsigned int address, DMXchannelType type = DMX_CHANNEL_BRIGHTNESS, bool width16bit = false, bool inverted = false, unsigned int minValue = 0, unsigned int maxValue=255)
    {
        this->address = address;
        this->type = type;
        this->width16bit = width16bit;
        this->minValue = minValue;
        this->maxValue = maxValue;
        this->inverted = inverted;
    };

    DMXchannelType type;
    unsigned int address;
    unsigned int minValue;
    unsigned int maxValue;
    bool width16bit;
    bool inverted;

};

class DMXfixture : public ofLight
{
    static bool oladSetup;
public:

#ifdef USE_OLA_LIB_AND_NOT_OSC
    static ola::DmxBuffer * buffer;
    static ola::StreamingClient * ola_client;
#else
    static ofxOscSender * oscSender;
    static int * buffer;
#endif

    DMXfixture()
    {
        if(!oladSetup)
        {
#ifdef USE_OLA_LIB_AND_NOT_OSC
            ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
            // Setup the client, this connects to the server
            if (!ola_client->Setup())
            {
                std::cerr << "OLA Setup failed" << std::endl;
            }
#else
            oscSender->setup("localhost", 7770);
#endif
            oladSetup = true;
        }
        addMe();
    };

    ~DMXfixture()
    {
        if(!DMXfixtures->empty()){
            removeMe();
        }
    };

    vector <DMXchannel*> DMXchannels;
    int DMXstartAddress;

    void setNormalisedBrightness(float brightness)
    {
        ofFloatColor c = ofLight::getDiffuseColor();
        c.setBrightness(brightness);
        ofLight::setDiffuseColor(c);
    };

    float getNormalisedBrightness()
    {
        return ofLight::getDiffuseColor().getBrightness();
    };

    void setTemperature(unsigned int degreesKelvin)
    {
        temperature = degreesKelvin;
        ofFloatColor c = DMXfixture::temperatureToColor(temperature);
        c.setBrightness(getNormalisedBrightness());
        ofLight::setDiffuseColor(c);
    };

    unsigned int getTemperature()
    {
        return temperature;
    };

    static void update()
    {
#ifdef USE_OLA_LIB_AND_NOT_OSC
        buffer->Blackout();
#endif
        for(vector<DMXfixture*>::iterator it = DMXfixtures->begin(); it != DMXfixtures->end(); it++)
        {
            DMXfixture * f = *(it);

            for(std::vector<DMXchannel*>::iterator chIt = f->DMXchannels.begin(); chIt != f->DMXchannels.end(); chIt++)
            {

                DMXchannel* c = *(chIt);

                // set the normalised value as a float

                float value = 0;
                if(c->type == DMXchannel::DMX_CHANNEL_BRIGHTNESS)
                {
                    value = f->getNormalisedBrightness();
                }
                if(c->type == DMXchannel::DMX_CHANNEL_RED)
                {
                    value = f->ofLight::getDiffuseColor().r;
                }
                if(c->type == DMXchannel::DMX_CHANNEL_GREEN)
                {
                    value = f->ofLight::getDiffuseColor().g;
                }
                if(c->type == DMXchannel::DMX_CHANNEL_BLUE)
                {
                    value = f->ofLight::getDiffuseColor().b;
                }
                if(c->type == DMXchannel::DMX_CHANNEL_HUE)
                {
                    value = f->ofLight::getDiffuseColor().getHue();
                }
                if(c->type == DMXchannel::DMX_CHANNEL_SATURATION)
                {
                    value = f->ofLight::getDiffuseColor().getSaturation();
                }
                if(c->type == DMXchannel::DMX_CHANNEL_COLOR_TEMPERATURE)
                {
                    value = ofMap(f->getTemperature(), f->temperatureRangeWarmKelvin, f->temperatureRangeColdKelvin, 0, 1.);
                }
                if(c->type == DMXchannel::DMX_CHANNEL_CW)
                {
                    value = ofMap(f->getTemperature(), f->temperatureRangeColdKelvin, f->temperatureRangeWarmKelvin, 0, 1.);
                    value = fminf(1.,ofMap(value, 0 , 0.5, 0., 1.));
                    value *= f->getNormalisedBrightness();
                }
                if(c->type == DMXchannel::DMX_CHANNEL_WW)
                {
                    value = ofMap(f->getTemperature(), f->temperatureRangeWarmKelvin, f->temperatureRangeColdKelvin, 0, 1.);
                    value = fminf(1.,ofMap(value, 0 , 0.5, 0., 1.));
                    value *= f->getNormalisedBrightness();
                }

                if(c->inverted){
                    value = 1.0-value;
                }

                // set int channel value as 8 or 16 bit;

                if(c->width16bit)
                {

                    unsigned int valueInt = ofMap(value, 0.,1., 0, 65025);

                    int highByte = valueInt/255;
                    updateChannelValue(c->address, highByte);
                    int lowByte = valueInt%255;
                    updateChannelValue(c->address+1, highByte);

                }
                else
                {
                    unsigned int valueInt = ofMap(value, 0.,1., c->minValue, c->maxValue);
                    updateChannelValue(c->address, valueInt);
                }
            }
        }

#ifdef USE_OLA_LIB_AND_NOT_OSC
        if (!ola_client->SendDmx(0, *(buffer)))
        {
            cout << "Send DMX failed" << endl;
        }
#endif
    };

#ifdef USE_OLA_LIB_AND_NOT_OSC
    static void updateChannelValue(int channel, int value)
    {
        buffer->SetChannel(channel-1, value);
    };
#else
    static void updateChannelValue(int channel, int value)
    {
        if(buffer[channel-1] != value)
        {
            ofxOscMessage m;
            m.setAddress("/dmx/universe/0");
            m.addIntArg(channel);
            m.addIntArg(value);
            oscSender->sendMessage(m);
            buffer[channel-1] = value;
        }
    };
#endif // USE_OLA_LIB_AND_NOT_OSC

    void draw()
    {
        ofPushStyle();
        ofSetColor(ofLight::getDiffuseColor());
        ofLight::draw();
        ofDrawBitmapString(ofToString(DMXstartAddress), ofLight::getGlobalPosition());
        ofPopStyle();
    }

    unsigned int temperatureRangeColdKelvin;
    unsigned int temperatureRangeWarmKelvin;

    static ofFloatColor temperatureToColor(unsigned int temp)
    {

        float blackbodyColor[91*3] =
        {
            1.0000, 0.0425, 0.0000, // 1000K
            1.0000, 0.0668, 0.0000, // 1100K
            1.0000, 0.0911, 0.0000, // 1200K
            1.0000, 0.1149, 0.0000, // ...
            1.0000, 0.1380, 0.0000,
            1.0000, 0.1604, 0.0000,
            1.0000, 0.1819, 0.0000,
            1.0000, 0.2024, 0.0000,
            1.0000, 0.2220, 0.0000,
            1.0000, 0.2406, 0.0000,
            1.0000, 0.2630, 0.0062,
            1.0000, 0.2868, 0.0155,
            1.0000, 0.3102, 0.0261,
            1.0000, 0.3334, 0.0379,
            1.0000, 0.3562, 0.0508,
            1.0000, 0.3787, 0.0650,
            1.0000, 0.4008, 0.0802,
            1.0000, 0.4227, 0.0964,
            1.0000, 0.4442, 0.1136,
            1.0000, 0.4652, 0.1316,
            1.0000, 0.4859, 0.1505,
            1.0000, 0.5062, 0.1702,
            1.0000, 0.5262, 0.1907,
            1.0000, 0.5458, 0.2118,
            1.0000, 0.5650, 0.2335,
            1.0000, 0.5839, 0.2558,
            1.0000, 0.6023, 0.2786,
            1.0000, 0.6204, 0.3018,
            1.0000, 0.6382, 0.3255,
            1.0000, 0.6557, 0.3495,
            1.0000, 0.6727, 0.3739,
            1.0000, 0.6894, 0.3986,
            1.0000, 0.7058, 0.4234,
            1.0000, 0.7218, 0.4485,
            1.0000, 0.7375, 0.4738,
            1.0000, 0.7529, 0.4992,
            1.0000, 0.7679, 0.5247,
            1.0000, 0.7826, 0.5503,
            1.0000, 0.7970, 0.5760,
            1.0000, 0.8111, 0.6016,
            1.0000, 0.8250, 0.6272,
            1.0000, 0.8384, 0.6529,
            1.0000, 0.8517, 0.6785,
            1.0000, 0.8647, 0.7040,
            1.0000, 0.8773, 0.7294,
            1.0000, 0.8897, 0.7548,
            1.0000, 0.9019, 0.7801,
            1.0000, 0.9137, 0.8051,
            1.0000, 0.9254, 0.8301,
            1.0000, 0.9367, 0.8550,
            1.0000, 0.9478, 0.8795,
            1.0000, 0.9587, 0.9040,
            1.0000, 0.9694, 0.9283,
            1.0000, 0.9798, 0.9524,
            1.0000, 0.9900, 0.9763,
            1.0000, 1.0000, 1.0000, /* 6500K */
            0.9771, 0.9867, 1.0000,
            0.9554, 0.9740, 1.0000,
            0.9349, 0.9618, 1.0000,
            0.9154, 0.9500, 1.0000,
            0.8968, 0.9389, 1.0000,
            0.8792, 0.9282, 1.0000,
            0.8624, 0.9179, 1.0000,
            0.8465, 0.9080, 1.0000,
            0.8313, 0.8986, 1.0000,
            0.8167, 0.8895, 1.0000,
            0.8029, 0.8808, 1.0000,
            0.7896, 0.8724, 1.0000,
            0.7769, 0.8643, 1.0000,
            0.7648, 0.8565, 1.0000,
            0.7532, 0.8490, 1.0000,
            0.7420, 0.8418, 1.0000,
            0.7314, 0.8348, 1.0000,
            0.7212, 0.8281, 1.0000,
            0.7113, 0.8216, 1.0000,
            0.7018, 0.8153, 1.0000,
            0.6927, 0.8092, 1.0000,
            0.6839, 0.8032, 1.0000,
            0.6755, 0.7975, 1.0000,
            0.6674, 0.7921, 1.0000,
            0.6595, 0.7867, 1.0000,
            0.6520, 0.7816, 1.0000,
            0.6447, 0.7765, 1.0000,
            0.6376, 0.7717, 1.0000,
            0.6308, 0.7670, 1.0000,
            0.6242, 0.7623, 1.0000,
            0.6179, 0.7579, 1.0000,
            0.6117, 0.7536, 1.0000,
            0.6058, 0.7493, 1.0000,
            0.6000, 0.7453, 1.0000,
            0.5944, 0.7414, 1.0000 /* 10000K */
        };

        float alpha = (temp % 100) / 100.0;
        int temp_index = ((temp - 1000) / 100)*3;

        ofFloatColor fromColor = ofFloatColor(blackbodyColor[temp_index], blackbodyColor[temp_index+1], blackbodyColor[temp_index+2]);
        ofFloatColor toColor = ofFloatColor(blackbodyColor[temp_index+3], blackbodyColor[temp_index+3+1], blackbodyColor[temp_index+3+2]);

        return fromColor.lerp(toColor, alpha);
    };

protected:

    unsigned int temperature;

    static vector<DMXfixture*> * DMXfixtures;

    void addMe()
    {
        DMXfixtures->push_back(this);
    }

    void removeMe()
    {
        if(DMXfixtures->size() == 1){
            DMXfixtures->clear();
        } else {
            for(vector<DMXfixture*>::iterator it = DMXfixtures->begin(); it != DMXfixtures->end(); ++it )
            {
                DMXfixture * l = *(it);
                if(this == l)
                {
                    DMXfixtures->erase(it);
                    break;
                }
            }
        }
    }

};

class ofxOlaShaderLight : public DMXfixture
{

public:

    static ofxUboShader * shader;

    enum shadingType {
        OFX_OLA_SHADER_LIGHT_PHONG,
        OFX_OLA_SHADER_LIGHT_GOURAUD,
        OFX_OLA_SHADER_LIGHT_FLAT
    };

    ofxOlaShaderLight()
    {
        if (!shaderSetup)
        {
            shader->load("shaders/phongShading");
            //shader->printLayout("Material");
            //shader->printLayout("Light");
            shaderSetup = true;
        }
    };

    ~ofxOlaShaderLight()
    {
        if(DMXfixture::DMXfixtures->empty())
        {
            shaderSetup = false;
        }
    }

    void setupBrightnessDMXChannel(int startAddress)
    {
        DMXstartAddress = startAddress;
        if(startAddress > 0)
        {
            DMXchannels.push_back(new DMXchannel(startAddress, DMXchannel::DMX_CHANNEL_BRIGHTNESS, false));
        }
    }

    struct Material
    {
        ofVec4f diffuseColor;
        ofVec4f specularColor;
        float specularShininess;
    };

    struct PerLight
    {
        ofVec3f cameraSpaceLightPos;
        ofVec4f lightIntensity;
        float lightAttenuation;
    };

    struct Light
    {
        ofVec4f ambientIntensity;
        int numberLights;
        PerLight lights[MAX_SHADER_LIGHTS];
    };
    
    struct NoisePoints
    {
        int numberOfPoints;
        float globalScale;
        float time;
        ofVec4f points [100];
    };

    static void begin()
    {
        if(shaderSetup)
        {
            shader->begin();
            updateShader();
            enabled = true;
        }
    }

    static void end()
    {
        if(shaderSetup)
        {
            glShadeModel(GL_SMOOTH);
            shader->end();
            enabled = false;
        }
    }

    static void setMaterial(Material m)
    {
        if (shaderSetup)
        {
            shader->setUniformBuffer("Material",m);
        }
    }
    
    static void setNoisePoints(NoisePoints n)
    {
        if (shaderSetup)
        {
            shader->setUniformBuffer("NoisePoints",n);
        }
    }

    static bool isEnabled(){
        return enabled;
    }

    static void setShadingType(shadingType s){
        shading = s;
    }

protected:

    static Light lightStruct;

    static void updateShaderLightStruct()
    {

        GLfloat cc[4];
        glGetFloatv(GL_LIGHT_MODEL_AMBIENT, cc);

        //TODO: use cc

        lightStruct.ambientIntensity = ofVec4f(0.0,0.0,0.0,1.0);
        lightStruct.numberLights = DMXfixtures->size();
        int lightIndex = 0;
        for(vector<DMXfixture*>::iterator it = DMXfixtures->begin(); it != DMXfixtures->end(); ++it )
        {
            DMXfixture * l = *(it);
            if(lightIndex < MAX_SHADER_LIGHTS)
            {
                ofFloatColor c = l->getDiffuseColor();
                lightStruct.lights[lightIndex].lightIntensity = ofVec4f(c[0],c[1],c[2],c[3]);
                lightStruct.lights[lightIndex].lightAttenuation = l->getAttenuationConstant();
                ofVec3f lightCamSpacePos = l->getPosition() * ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
                lightStruct.lights[lightIndex].cameraSpaceLightPos = lightCamSpacePos;
            }
            else
            {
                ofLog(OF_LOG_ERROR, "ofxOlaShaderLights: There are more lights than MAX_SHADER_LIGHTS");
            }
            lightIndex++;
        }
    };

    static void updateShader()
    {
        if (shaderSetup)
        {
            updateShaderLightStruct();
            shader->setUniformBuffer("Light", lightStruct);

            switch (shading) {
                case OFX_OLA_SHADER_LIGHT_FLAT:
                    shader->setUniform1i("flatShading", 2);
                    break;
                case OFX_OLA_SHADER_LIGHT_GOURAUD:
                    shader->setUniform1i("flatShading", 1);
                    break;
                case OFX_OLA_SHADER_LIGHT_PHONG:
                    shader->setUniform1i("flatShading", 0);
                    break;

                default:
                    break;
            }
        }
    }

    static bool shaderSetup;

    static bool enabled;

    static shadingType shading;

};
