//CSCI 5607 HW 2 - Image Conversion Instructor: S. J. Guy <sjguy@umn.edu>
//In this assignment you will load and convert between various image formats.
//Additionally, you will manipulate the stored image data by quantizing, cropping, and supressing channels

#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <fstream>
using namespace std;

/**
 * Image
 **/
Image::Image (int width_, int height_){

    assert(width_ > 0);
    assert(height_ > 0);

    width           = width_;
    height          = height_;
    num_pixels      = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;
    
    data.raw = new uint8_t[num_pixels*4];
		int b = 0; //which byte to write to
		for (int j = 0; j < height; j++){
			for (int i = 0; i < width; i++){
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
			}
		}

    assert(data.raw != NULL);
}

//Image::Image (const Image& src){
//	width           = src.width;
//	height          = src.height;
//	num_pixels      = width * height;
//	sampling_method = IMAGE_SAMPLING_POINT;
//
//	data.raw = new uint8_t[num_pixels*4];
//
//	//memcpy(data.raw, src.data.raw, num_pixels);
//	*data.raw = *src.data.raw;
//}
Image::Image (const Image& src){
   width = src.width;
   height = src.height;
   num_pixels = width * height;
   sampling_method =
   IMAGE_SAMPLING_POINT;
   data.raw = new uint8_t[num_pixels*4];
   memcpy(data.raw, src.data.raw, num_pixels*4); //NEW, use this to copy
}

Image::Image (char* fname){

	int lastc = strlen(fname);
   int numComponents; //(e.g., Y, YA, RGB, or RGBA)
   data.raw = stbi_load(fname, &width, &height, &numComponents, 4);
	
	if (data.raw == NULL){
		printf("Error loading image: %s", fname);
		exit(-1);
	}
	
	num_pixels = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;
	
}

Image::~Image (){
    delete data.raw;
    data.raw = NULL;
}

void Image::Write(char* fname){
	
	int lastc = strlen(fname);

	switch (fname[lastc-1]){
	   case 'g': //jpeg (or jpg) or png
	     if (fname[lastc-2] == 'p' || fname[lastc-2] == 'e') //jpeg or jpg
	        stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
	     else //png
	        stbi_write_png(fname, width, height, 4, data.raw, width*4);
	     break;
	   case 'a': //tga (targa)
	     stbi_write_tga(fname, width, height, 4, data.raw);
	     break;
	   case 'p': //bmp
	   default:
	     stbi_write_bmp(fname, width, height, 4, data.raw);
	}
}


void Image::Brighten (double factor){
	int x,y;
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = p*factor;
			GetPixel(x,y) = scaled_p;
		}
	}
}

void Image::ExtractChannel(int channel){
   for (int x = 0 ; x < Width(); x++){
      for (int y = 0 ; y < Height(); y++){
         Pixel p = GetPixel(x,y);
         switch (channel) {
            case 0:
               GetPixel(x,y).Set(p.r, 0, 0);
               break;
            case 1:
               GetPixel(x,y).Set(0, p.g, 0);
               break;
            case 2:
               GetPixel(x,y).Set(0, 0, p.b);
               break;
            default:
               break;
         }
      }
   }
}


void Image::Quantize (int nbits){
   for (int x = 0; x < Width(); x++) {
      for (int y = 0; y < Height(); y++) {
         GetPixel(x,y) = PixelQuant(GetPixel(x,y), nbits);
      }
   }
}

Image* Image::Crop(int x, int y, int w, int h){
   Image* crop = new Image(w, h);
   int i, j, xPos, yPos;
   for (xPos = x, i = 0; xPos < x + w; xPos++) {
      for (yPos = y, j = 0; yPos < y + h; yPos++) {
         crop->GetPixel(i, j) = GetPixel(xPos,yPos);
         j++;
      }
      i++;
   }
   return crop;
}

// TODO: seems funky - find different way of adding noise
void Image::AddNoise (double factor){
   int x, y;
   // salt and pepper noise; okay, but looks a bit funky
//   for (x = 0; x < Width(); x++) {
//      for (y = 0; y < Height(); y++) {
//         double prob = rand() % 100 + 1;
//         if (prob / 100 < factor) {
//            if (rand() % 2 == 0) {
//               GetPixel(x,y) = Pixel(0,0,0);
//            } else {
//               GetPixel(x,y) = Pixel(255, 255, 255);
//            }
//         }
//      }
//   }
   
   // color noise
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
         Pixel noise = Pixel((rand() % 256) * factor, (rand() % 256) * factor, (rand() % 256) * factor);
//         GetPixel(x,y) = GetPixel(x,y) + noise;  // this makes things brighter, dumbass
         GetPixel(x,y) = GetPixel(x,y)*(1-factor) + noise;
      }
   }
}

void Image::ChangeContrast (double factor){
   // find average gray
   int x, y;
   float gray = 0;
   
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
         Pixel p = GetPixel(x,y);
//         gray += (0.3*p.r + 0.6*p.g + 0.1*p.b);v
         gray += p.Luminance();
      }
   }
   gray /= (Width() * Height());
   Pixel grayP = Pixel(gray, gray, gray);
   
   // scale deviation for each pixel
//   float val, dev;
//   float l, xx, d, sd;  // luminance, scale factor, deviation, scaled deviation
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
//         Pixel p = GetPixel(x,y);
//         val = (0.3*p.r + 0.6*p.g + 0.1*p.b);
////         dev = val - gray;
//
//         dev = 1 + ((val - gray)/gray);
//         GetPixel(x, y).SetClamp(p.r*dev, p.g*dev, p.b*dev);
         
//         l = p.Luminance();
//         d = l - gray;
//         sd = d * factor;
//
//         xx = ((sd + gray) / l);
//
//         GetPixel(x,y).SetClamp(p.r*xx, p.g*xx, p.b*xx);
         
         // hey look what i found
         
         // I assume this interpolates p to grayP?? the function's a
            // bit confusing. no idea what t does but it seems to
            // work like this (does NOT work if you just pass factor)
         GetPixel(x,y) = PixelLerp(GetPixel(x,y), grayP, 1-factor);
      }
   }
}


void Image::ChangeSaturation(double factor){
   int x, y;
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
         float l = GetPixel(x,y).Luminance();
         Pixel gray = Pixel(l, l, l);
         GetPixel(x,y) = PixelLerp(GetPixel(x,y), gray, 1-factor);
      }
   }
}


//For full credit, check that your dithers aren't making the pictures systematically brighter or darker
void Image::RandomDither (int nbits){
   // 2 ^ (bits shrunk by)
   float h = pow(2, 8-nbits);
   AddNoise(0.004 * h); // this seems to look the best? not sure if it's right
   Quantize(nbits);
}

//This bayer method gives the quantization thresholds for an ordered dither.
//This is a 4x4 dither pattern, assumes the values are quantized to 16 levels.
//You can either expand this to a larger bayer pattern. Or (more likely), scale
//the threshold based on the target quantization levels.
static int Bayer4[4][4] ={
    {15,  7, 13,  5},
    { 3, 11,  1,  9},
    {12,  4, 14,  6},
    { 0,  8,  2, 10}
};


void Image::OrderedDither(int nbits){
	/* WORK HERE */
}

/* Error-diffusion parameters */
const double
    ALPHA = 7.0 / 16.0,
    BETA  = 3.0 / 16.0,
    GAMMA = 5.0 / 16.0,
    DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits){
   int x, y;
   for (y = 0; y < Height(); y++) {
      for (x = 0; x < Width(); x++) {
         Pixel p = GetPixel(x,y);
         Pixel qP = PixelQuant(p, nbits);
//         Pixel error = Pixel(p.r - qP.r, p.g - qP.g, p.b - qP.b);
         Pixel error;
         error.SetClamp(p.r - qP.r, p.g - qP.g, p.b - qP.b);
         GetPixel(x,y) = qP;

         if (ValidCoord(x+1, y)) {
//            GetPixel(x+1, y) = GetPixel(x+1, y) + error * ALPHA;
            Pixel cur = GetPixel(x+1, y);
            GetPixel(x+1, y).SetClamp(cur.r + error.r * ALPHA,
                                      cur.g + error.g * ALPHA,
                                      cur.b + error.b * ALPHA);
         }

         if (ValidCoord(x-1, y+1)) {
//            GetPixel(x-1, y+1) = GetPixel(x-1, y+1) + error * BETA;
            Pixel cur = GetPixel(x-1, y+1);
            GetPixel(x-1, y+1).SetClamp(cur.r + error.r * BETA,
                                      cur.g + error.g * BETA,
                                      cur.b + error.b * BETA);
         }

         if (ValidCoord(x, y+1)) {
//            GetPixel(x, y+1) = GetPixel(x, y+1) + error * GAMMA;
            Pixel cur = GetPixel(x, y+1);
            GetPixel(x, y+1).SetClamp(cur.r + error.r * GAMMA,
                                      cur.g + error.g * GAMMA,
                                      cur.b + error.b * GAMMA);
         }

         if (ValidCoord(x+1, y+1)) {
//            GetPixel(x+1, y+1) = GetPixel(x+1, y+1) + error * DELTA;
            Pixel cur = GetPixel(x+1, y+1);
            GetPixel(x+1, y+1).SetClamp(cur.r + error.r * DELTA,
                                      cur.g + error.g * DELTA,
                                      cur.b + error.b * DELTA);
         }
      }
   }
}

void Image::Blur(int n){
   float r, g, b; //I got better results converting everything to floats, then converting back to bytes
   int x, y, i, j;
   Image* img_copy = new Image(*this); //This is will copying the image, so you can read the orginal values for filtering (
                                          //  ... don't forget to delete the copy!

   // calculate gaussian matrix
   int N = 2 * n + 1;
   float sigma = N / 2.0f;
   float sum = 0;
   float gauss[N][N];
   
   for (x = 0; x < N; x++) {
      for (y = 0; y < N; y++) {
         float a = 1.0f / (2 * M_PI * pow(sigma, 2));
         float b = -(pow((N/2)-x,2) + pow((N/2)-y,2)) / (2.0f*pow(sigma,2));
         gauss[x][y] = a * exp(b);
         sum += gauss[x][y];
      }
   }
   
   // normalize
   for (x = 0; x < N; x++) {
      for (y = 0; y < N; y++) {
         gauss[x][y] /= sum;
      }
   }
   
   // convolve
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
         r=g=b=0;
         for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
               // clamp x/y values to bounds of image
               // for some reason it doesn't recognize the std::clamp function
               // so here we are
               // but it should extend the filter to the image bounds
               int xClamp = x + i - N/2;
               if (xClamp < 0) xClamp = 0;
               else if (xClamp >= Width()) xClamp = Width()-1;
               
               int yClamp = y + j - N/2;
               if (yClamp < 0) yClamp = 0;
               else if (yClamp >= Height()) yClamp = Height()-1;
               
               Pixel p = GetPixel(xClamp, yClamp);
               r += p.r*gauss[i][j];
               g += p.g*gauss[i][j];
               b += p.b*gauss[i][j];
            }
         }
         img_copy->GetPixel(x,y).SetClamp(r,g,b);
      }
   }
   this->data.raw = img_copy->data.raw;
//   delete img_copy;  // this causes Issues
}

void Image::Sharpen(int n){
	// extrapolate from blurred image - use pixellerp?
   Image* img_copy = new Image(*this);
   img_copy->Blur(n);
   
   int x, y;
   for (x = 0; x < Width(); x++) {
      for (y = 0; y < Height(); y++) {
         Pixel p = GetPixel(x,y);
         Pixel b = img_copy -> GetPixel(x,y);
         GetPixel(x,y) = PixelLerp(b, p, n);
      }
   }
}

void Image::EdgeDetect(){  // sobel
   int x, y, i, j;
   float rx, gx, bx, ry, gy, by;
   Image* img = new Image(*this);
   
   int sobelX[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
   int sobelY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
   
   for (x = 1; x < Width() - 1; x++) {
      for (y = 1; y < Height() - 1; y++) {
         rx=gx=bx=ry=gy=by=0;
         for (i = -1; i <= 1; i++) {
            for (j = -1; j <= 1; j++) {
               Pixel p = GetPixel(x+i, y+j);
               rx += p.r*sobelX[1+i][1+j];
               gx += p.g*sobelX[1+i][1+j];
               bx += p.b*sobelX[1+i][1+j];
               
               ry += p.r*sobelY[1+i][1+j];
               gy += p.g*sobelY[1+i][1+j];
               by += p.b*sobelY[1+i][1+j];
            }
         }
         
         img->GetPixel(x,y).SetClamp(sqrt(pow(rx, 2) + pow(ry,2)) + 0.5,
                                    sqrt(pow(gx, 2) + pow(gy,2)) + 0.5,
                                    sqrt(pow(bx, 2) + pow(by,2)) + 0.5);
      }
   }
   
   
   this->data.raw = img->data.raw;
//   delete img;
}

Image* Image::Scale(double sx, double sy){
	/* WORK HERE */
	return NULL;
}

Image* Image::Rotate(double angle){
	/* WORK HERE */
	return NULL;
}

void Image::Fun(){
	/* WORK HERE */
}

/**
 * Image Sample
 **/
void Image::SetSamplingMethod(int method){
   assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
   sampling_method = method;
}


Pixel Image::Sample (double u, double v){
   /* WORK HERE */
   return Pixel();
}
