#ifndef DRCOLOR_H
#define DRCOLOR_H
#include <math.h>

#include "../log/logger.h"



namespace DevRelief {

    enum HUE {
        RED= 0,
        ORANGE= 30,
        YELLOW= 60,
        GREEN= 90,
        CYAN= 180,
        BLUE= 200,
        MAGENTA= 285,
        PURPLE= 315
    };

    int clamp(int min, int max, int val) {
        if (val < min) { return min;}
        if (val>max) { return max;}
        return val;
    }

    double max3(double a,double b, double c) {
        if (a>b) {
            return a>c ? a : c;
        } else {
            return b>c ? b : c;
        }
    }
    

    double min3(double a,double b, double c) {
        if (a<b) {
            return a<c ? a : c;
        } else {
            return b<c ? b : c;
        }
    }
    
    class CRGB {
        public: 
            CRGB() {
                red = 0;
                green = 0;
                blue = 0;
            }

            CRGB(const CRGB& other) {
                red = other.red;
                blue = other.blue;
                green = other.green;
            }
            
            CRGB(uint8_t r,uint8_t g,uint8_t b) {
                red = r;
                green = g;
                blue = b;
            }

          

        public:
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            
    };

    class CHSL {
        public: 
            CHSL() {
                hue = 0;
                saturation = 0;
                lightness = 0;
            }
            
            CHSL(int16_t hue,int16_t saturation,int16_t lightness) {
                this->hue = clamp(0,360,hue);
                this->saturation = clamp(0,100,saturation);
                this->lightness = clamp(0,100,lightness);
            }

            
        public:
            uint16_t hue;  // 0-360
            uint16_t saturation; // 0-100
            uint16_t lightness;  // 0-100
            
    };

 
    CHSL RGBToHSL(const CRGB&rgb)
    {
       // double r = rgb.red/255.0;
       // double g = rgb.green/255.0;
        double r = rgb.red/255.0;
        double g = rgb.green/255.0;
        double b = rgb.blue/255.0;
        double hue, sat;

        
    
        double maxValue , minValue;
        
        maxValue = max3(r,g,b);
        
        minValue=min3(r,g,b);
        
        double h, s, l = (maxValue + minValue) / 2;

        if(maxValue == minValue){
            h = s = 0; 
        }else{
            double d = maxValue - minValue;
            s = l > 0.5 ? d / (2 - maxValue - minValue) : d / (maxValue + minValue);
            if (maxValue == r) {
                h = (g - b) / d + (g < b ? 6 : 0); 
            } else if (maxValue == g) {
                h = (b - r) / d + 2;
            } else {
                 h = (r - g) / d + 4;
            }
            h /= 6;
        }
        return CHSL(h*360,s*100,l*100);
    }

    float HueToRGB(float v1, float v2, float vH) {
        if (vH < 0)
            vH += 1;

        if (vH > 1)
            vH -= 1;

        if ((6 * vH) < 1)
            return (v1 + (v2 - v1) * 6 * vH);

        if ((2 * vH) < 1)
            return v2;

        if ((3 * vH) < 2)
            return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

        return v1;
    }

    CRGB HSLToRGB(const CHSL& hsl) {
        //m_logger->debug(hsl to rgb (%d,%d,%d)",(int)hsl.hue,(int)hsl.saturation,(int)hsl.lightness);
        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;

        float h = hsl.hue/360.0;
        float s = 1.0*hsl.saturation/100.0;
        float l = 1.0*hsl.lightness/100.0;
        if (s == 0)
        {
            r = g = b = (unsigned char)(l * 255);
        }
        else
        {
            float v1, v2;
            float hue = (float)h;

            v2 = (l < 0.5) ? (l * (1 + s)) : ((l + s) - (l * s));
            v1 = 2 * l - v2;

            r = (unsigned char)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
            g = (unsigned char)(255 * HueToRGB(v1, v2, hue));
            b = (unsigned char)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
        }

        CRGB rgb(r, g, b);
        //m_logger->debug("hsl (%f,%f,%f)->rgb(%d,%d,%d)",(int)h,s,l,rgb.red,rgb.green,rgb.blue);
        return rgb;
    }


    
}


#endif 