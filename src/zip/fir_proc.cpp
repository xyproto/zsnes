/*
 * This program is  free software; you can redistribute it  and modify it
 * under the terms of the GNU  General Public License as published by the
 * Free Software Foundation; either version 2  of the license or (at your
 * option) any later version.
 *
 * Authors: Markus Fick <webmaster@mark-f.de> fir-resampler
 *          Chris Moeller <chris@kode54.net> C/C++ fir_interpolate functions based off original macros
 *          
*/

#ifdef __LINUX__
#include "../gblhdr.h"
#else
#include <math.h>
#include <stdio.h>
#endif

#ifndef log2
inline float log2(float f) {
  float t;
#ifdef __GNUC__
  __asm__ ("
    fld1\n
    fxch\n
    fyl2x\n
    fst (%1)"
  :
  : "st" (f), "r" (&t)
  );
#else
  __asm {
    fld1
    fld f
    fyl2x
    fstp t
  }
#endif
  return t;
}
#endif

/* 
  ------------------------------------------------------------------------------------------------
   fir interpolation doc,
	(derived from "an engineer's guide to fir digital filters", n.j. loy)

	calculate coefficients for ideal lowpass filter (with cutoff = fc in 0..1 (mapped to 0..nyquist))
	  c[-N..N] = (i==0) ? fc : sin(fc*pi*i)/(pi*i)

	then apply selected window to coefficients
	  c[-N..N] *= w(0..N)
	with n in 2*N and w(n) being a window function (see loy)

	then calculate gain and scale filter coefs to have unity gain.
  ------------------------------------------------------------------------------------------------
*/
// quantizer scale of window coefs
#define WFIR_QUANTBITS		14
#define WFIR_QUANTSCALE		(1L<<WFIR_QUANTBITS)
#define WFIR_8SHIFT			(WFIR_QUANTBITS-8)
#define WFIR_16BITSHIFT		(WFIR_QUANTBITS)
// log2(number)-1 of precalculated taps range is [4..12]
#define WFIR_FRACBITS		10
#define WFIR_LUTLEN			((1L<<(WFIR_FRACBITS+1))+1)
// number of samples in window
#define WFIR_LOG2WIDTH		3
#define WFIR_WIDTH			(1L<<WFIR_LOG2WIDTH)
#define WFIR_SMPSPERWING	((WFIR_WIDTH-1)>>1)
// cutoff (1.0 == pi/2)
#define WFIR_CUTOFF			0.90f
#define WFIR_CUTOFFBITS		12
#define WFIR_CUTOFFLEN		(1L<<(WFIR_CUTOFFBITS))
// wfir type
#define WFIR_HANN			0
#define WFIR_HAMMING		1
#define WFIR_BLACKMANEXACT	2
#define WFIR_BLACKMAN3T61	3
#define WFIR_BLACKMAN3T67	4
#define WFIR_BLACKMAN4T92	5
#define WFIR_BLACKMAN4T74	6
#define WFIR_KAISER4T		7
#define WFIR_TYPE			WFIR_BLACKMANEXACT
// wfir help
#ifndef M_zPI
#define M_zPI			3.1415926535897932384626433832795
#endif
#define M_zEPS			1e-8
#define M_zBESSELEPS	1e-21

class CzWINDOWEDFIR
{	public:
		CzWINDOWEDFIR( );
		~CzWINDOWEDFIR( );
		float coef( int _PCnr, float _POfs, float _PCut, int _PWidth, int _PType ) //float _PPos, float _PFc, int _PLen )
		{	double	_LWidthM1		= _PWidth-1;
			double	_LWidthM1Half	= 0.5*_LWidthM1;
			double	_LPosU			= ((double)_PCnr - _POfs);
			double	_LPos			= _LPosU-_LWidthM1Half;
			double	_LPIdl			= 2.0*M_zPI/_LWidthM1;
			double	_LWc,_LSi;
			if( fabs(_LPos)<M_zEPS )
			{	_LWc	= 1.0;
				_LSi	= _PCut;
			}
			else
			{	switch( _PType )
				{	case WFIR_HANN:
						_LWc = 0.50 - 0.50 * cos(_LPIdl*_LPosU);
						break;
					case WFIR_HAMMING:
						_LWc = 0.54 - 0.46 * cos(_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMANEXACT:
						_LWc = 0.42 - 0.50 * cos(_LPIdl*_LPosU) + 0.08 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN3T61:
						_LWc = 0.44959 - 0.49364 * cos(_LPIdl*_LPosU) + 0.05677 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN3T67:
						_LWc = 0.42323 - 0.49755 * cos(_LPIdl*_LPosU) + 0.07922 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN4T92:
						_LWc = 0.35875 - 0.48829 * cos(_LPIdl*_LPosU) + 0.14128 * cos(2.0*_LPIdl*_LPosU) - 0.01168 * cos(3.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN4T74:
						_LWc = 0.40217 - 0.49703 * cos(_LPIdl*_LPosU) + 0.09392 * cos(2.0*_LPIdl*_LPosU) - 0.00183 * cos(3.0*_LPIdl*_LPosU);
						break;
					case WFIR_KAISER4T:
						_LWc = 0.40243 - 0.49804 * cos(_LPIdl*_LPosU) + 0.09831 * cos(2.0*_LPIdl*_LPosU) - 0.00122 * cos(3.0*_LPIdl*_LPosU);
						break;
					default:
						_LWc = 1.0;
						break;
				}
				_LPos	 *= M_zPI;
				_LSi	 = sin(_PCut*_LPos)/_LPos;
			}
			return (float)(_LWc*_LSi);
		}
		static signed short lut[WFIR_LUTLEN*WFIR_WIDTH];
		static signed int lut_co[WFIR_CUTOFFLEN*WFIR_WIDTH];
};

signed short CzWINDOWEDFIR::lut[WFIR_LUTLEN*WFIR_WIDTH];
signed int CzWINDOWEDFIR::lut_co[WFIR_CUTOFFLEN*WFIR_WIDTH];

CzWINDOWEDFIR::CzWINDOWEDFIR()
{	int _LPcl;
	float _LPcllen	= (float)(1L<<WFIR_FRACBITS);	// number of precalculated lines for 0..1 (-1..0)
	float _LNorm	= 1.0f / (float)(2.0f * _LPcllen);
	float _LCut		= WFIR_CUTOFF;
	float _LScale	= (float)WFIR_QUANTSCALE;
	float _LGain,_LCoefs[WFIR_WIDTH];
	for( _LPcl=0;_LPcl<WFIR_LUTLEN;_LPcl++ )
	{
		float _LOfs		= ((float)_LPcl-_LPcllen)*_LNorm;
		int _LCc,_LIdx	= _LPcl<<WFIR_LOG2WIDTH;
		for( _LCc=0,_LGain=0.0f;_LCc<WFIR_WIDTH;_LCc++ )
		{	_LGain	+= (_LCoefs[_LCc] = coef( _LCc, _LOfs, _LCut, WFIR_WIDTH, WFIR_TYPE ));
		}
		_LGain = 1.0f/_LGain;
		for( _LCc=0;_LCc<WFIR_WIDTH;_LCc++ )
		{	float _LCoef = (float)floor( 0.5 + _LScale*_LCoefs[_LCc]*_LGain );
			lut[_LIdx+_LCc] = (signed short)( (_LCoef<-_LScale)?-_LScale:((_LCoef>_LScale)?_LScale:_LCoef) );
		}
	}
	for( _LPcl=0;_LPcl<WFIR_CUTOFFLEN;_LPcl++ )
	{
		int _LCc,_LIdx	= _LPcl<<WFIR_LOG2WIDTH;
		_LCut = 1.0f - pow((float)_LPcl / WFIR_CUTOFFLEN,1/5.0f); // bleh  (1.0f + 5.0f * (log2(2.0f + (_LPcl / (WFIR_CUTOFFLEN / 256.0f))) - 1.0f));
		for(_LCc=0,_LGain=0.0f;_LCc<WFIR_WIDTH;_LCc++ )
		{
			_LGain	+= (_LCoefs[_LCc] = coef( _LCc, -0.5f, _LCut, WFIR_WIDTH, WFIR_TYPE ));
		}
		_LGain = 1.0f/_LGain;
		for( _LCc=0; _LCc<WFIR_WIDTH;_LCc++ )
		{
			float _LCoef = (float)floor( 0.5 + _LScale*_LCoefs[_LCc]*_LGain );
			lut_co[_LIdx+_LCc] = (signed int)( (_LCoef<-_LScale)?-_LScale:((_LCoef>_LScale)?_LScale:_LCoef) );
		}
	}
}

CzWINDOWEDFIR::~CzWINDOWEDFIR()
{	// nothing todo
}

/*
float coef( int _PCnr, float _POfs, float _PCut, int _PWidth, int _PType ) //float _PPos, float _PFc, int _PLen )
{
	double	_LWidthM1		= _PWidth-1;
	double	_LWidthM1Half	= 0.5*_LWidthM1;
	double	_LPosU			= ((double)_PCnr - _POfs);
	double	_LPos			= _LPosU-_LWidthM1Half;
	double	_LPIdl			= 2.0*M_zPI/_LWidthM1;
	double	_LWc,_LSi;
	if( fabs(_LPos)<M_zEPS )
	{	_LWc	= 1.0;
		_LSi	= _PCut;
	}
	else
	{	switch( _PType )
		{	case WFIR_HANN:
				_LWc = 0.50 - 0.50 * cos(_LPIdl*_LPosU);
				break;
			case WFIR_HAMMING:
				_LWc = 0.54 - 0.46 * cos(_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMANEXACT:
				_LWc = 0.42 - 0.50 * cos(_LPIdl*_LPosU) + 0.08 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN3T61:
				_LWc = 0.44959 - 0.49364 * cos(_LPIdl*_LPosU) + 0.05677 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN3T67:
				_LWc = 0.42323 - 0.49755 * cos(_LPIdl*_LPosU) + 0.07922 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN4T92:
				_LWc = 0.35875 - 0.48829 * cos(_LPIdl*_LPosU) + 0.14128 * cos(2.0*_LPIdl*_LPosU) - 0.01168 * cos(3.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN4T74:
				_LWc = 0.40217 - 0.49703 * cos(_LPIdl*_LPosU) + 0.09392 * cos(2.0*_LPIdl*_LPosU) - 0.00183 * cos(3.0*_LPIdl*_LPosU);
				break;
			case WFIR_KAISER4T:
				_LWc = 0.40243 - 0.49804 * cos(_LPIdl*_LPosU) + 0.09831 * cos(2.0*_LPIdl*_LPosU) - 0.00122 * cos(3.0*_LPIdl*_LPosU);
				break;
			default:
				_LWc = 1.0;
				break;
		}
		_LPos	 *= M_zPI;
		_LSi	 = sin(_PCut*_LPos)/_LPos;
	}
	return (float)(_LWc*_LSi);
}
*/

CzWINDOWEDFIR sfir;

extern "C" signed short *fir_lut = &CzWINDOWEDFIR::lut[0];


#if 0

// fir interpolation
#define WFIR_FRACSHIFT	(16-(WFIR_FRACBITS+1+WFIR_LOG2WIDTH))
#define WFIR_FRACMASK	((((1L<<(17-WFIR_FRACSHIFT))-1)&~((1L<<WFIR_LOG2WIDTH)-1)))
#define WFIR_FRACHALVE	(1L<<(16-(WFIR_FRACBITS+2)))

inline int __fir_interpolate(unsigned int nPos, int *p)
{
	int poshi  = nPos >> 24;
	int poslo  = ((nPos >> 8) & 0xFFFF);
	int firidx = ((poslo+WFIR_FRACHALVE)>>WFIR_FRACSHIFT) & WFIR_FRACMASK;
	int vol   = (CzWINDOWEDFIR::lut[firidx+0]*p[poshi+0]);
		vol  += (CzWINDOWEDFIR::lut[firidx+1]*p[poshi+1]);
		vol  += (CzWINDOWEDFIR::lut[firidx+2]*p[poshi+2]);
		vol  += (CzWINDOWEDFIR::lut[firidx+3]*p[poshi+3]);
		vol  += (CzWINDOWEDFIR::lut[firidx+4]*p[poshi+4]);
		vol  += (CzWINDOWEDFIR::lut[firidx+5]*p[poshi+5]);
		vol  += (CzWINDOWEDFIR::lut[firidx+6]*p[poshi+6]);
		vol  += (CzWINDOWEDFIR::lut[firidx+7]*p[poshi+7]);
	vol	>>= WFIR_16BITSHIFT;

	return vol;
}

extern "C" int fir_interpolate(unsigned int nPos, int *p)
{
	return __fir_interpolate(nPos, p);
}

#endif

#define WFIR_CUTOFFSHIFT	(32-(WFIR_CUTOFFBITS+WFIR_LOG2WIDTH))
#define WFIR_CUTOFFMASK	((((1L<<(32-WFIR_CUTOFFSHIFT))-1)&~((1L<<WFIR_LOG2WIDTH)-1)))
#define WFIR_CUTOFFHALVE	(1L<<(32-(WFIR_CUTOFFBITS+1)))

inline void __fir_downsample(unsigned int freq, signed int *p, signed short *out)
{
/*
	float cutoff = WFIR_CUTOFF / (1.0f + log2((float)freq / 16777216.0f));
	float _LGain, _LCoefs[WFIR_WIDTH];
	int _LCc;
	for(_LCc=0,_LGain=0.0f;_LCc<WFIR_WIDTH;_LCc++ )
	{
		_LGain	+= (_LCoefs[_LCc] = coef( _LCc, -0.5f, cutoff, WFIR_WIDTH, WFIR_TYPE ));
	}
	_LGain = 1.0f/_LGain;
	for(int ct=0;ct<16;ct++)
	{
		signed int vol;
		float acc = (_LCoefs[0] * _LGain * (float)p[ct+0]);
			acc += (_LCoefs[1] * _LGain * (float)p[ct+1]);
			acc += (_LCoefs[2] * _LGain * (float)p[ct+2]);
			acc += (_LCoefs[3] * _LGain * (float)p[ct+3]);
			acc += (_LCoefs[4] * _LGain * (float)p[ct+4]);
			acc += (_LCoefs[5] * _LGain * (float)p[ct+5]);
			acc += (_LCoefs[6] * _LGain * (float)p[ct+6]);
			acc += (_LCoefs[7] * _LGain * (float)p[ct+7]);

		vol = (signed int)acc;
		if (vol > 32767) vol=32767;
		else if (vol < -32768) vol=-32768;
		out[ct]=(signed short)vol;
	}
*/
	int firidx = (((freq-16777216)+WFIR_CUTOFFHALVE)>>WFIR_CUTOFFSHIFT) & WFIR_CUTOFFMASK;
	for(int ct=0;ct<16;ct++)
	{
		int vol   = (CzWINDOWEDFIR::lut_co[firidx+0]*p[ct+0]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+1]*p[ct+1]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+2]*p[ct+2]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+3]*p[ct+3]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+4]*p[ct+4]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+5]*p[ct+5]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+6]*p[ct+6]);
			vol  += (CzWINDOWEDFIR::lut_co[firidx+7]*p[ct+7]);
		vol	>>= WFIR_16BITSHIFT;
		if (vol > 32767) vol=32767;
		else if (vol < -32768) vol=-32768;
		out[ct]=(signed short)vol;
	}
}

extern "C" void fir_downsample(unsigned int freq, signed int *p, signed short *out)
{
	__fir_downsample(freq, p, out);
}