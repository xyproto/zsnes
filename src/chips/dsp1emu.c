//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define DebugDSP1

// uncomment some lines to test
//#define printinfo
//#define debug02
//#define debug0A
//#define debug06


#ifdef DebugDSP1

FILE * LogFile = NULL;

void Log_Message (char *Message, ...)
{
	char Msg[400];
	va_list ap;

   va_start(ap,Message);
   vsprintf(Msg,Message,ap );
   va_end(ap);
	
   strcat(Msg,"\r\n\0");
   fwrite(Msg,strlen(Msg),1,LogFile);
}

void Start_Log (void)
{
	char LogFileName[255];
	char *p;

   strcpy(LogFileName,"dsp1emu.log\0");
	
   LogFile = fopen(LogFileName,"wb");
}

void Stop_Log (void)
{
   if (LogFile)
   {
      fclose(LogFile);
      LogFile = NULL;
	}
}

#endif


/***************************************************************************\
*  Math tables                                                              *
\***************************************************************************/

int CosTable2[256]={65536,65516,65457,65358,65220,65043,64826,64571,64276,63943,63571,63161,62713,62227,61704,61144,60546,59912,59243,58537,57796,
57020,56210,55367,54489,53579,52637,51663,50658,49622,48556,47461,46338,45187,44008,42803,41572,
40316,39036,37732,36406,35057,33688,32298,30889,29461,28015,26553,25074,23581,22073,20552,19018,17473,15918,14353,12779,11198,9610,8016,6417,4814,
3209,1601,-6,-1615,-3222,-4828,-6430,-8029,-9623,-11211,-12792,-14366,-15931,-17486,-19031,-20565,-22086,-23593,-25087,-26565,-28028,-29473,
-30901,-32310,-33700,-35069,-36417,-37743,-39047,-40327,-41583,-42813,-44018,-45197,-46348,-47471,-48565,
-49631,-50666,-51671,-52645,-53587,-54497,-55374,-56217,-57027,-57802,-58543,-59248,-59918,-60551,-61148,
-61709,-62232,-62717,-63165,-63575,-63946,-64279,-64573,-64828,-65044,-65221,-65359,-65457,-65516,-65535,
-65515,-65456,-65357,-65219,-65041,-64824,-64568,-64273,-63940,-63568,-63158,-62709,-62223,-61699,-61139,
-60541,-59907,-59237,-58531,-57790,-57014,-56203,-55359,-54482,-53571,-52629,-51654,-50649,-49613,-48547,
-47452,-46328,-45177,-43998,-42793,-41562,-40306,-39025,-37721,-36395,-35046,-33676,-32286,-30877,-29449,
-28003,-26540,-25062,-23568,-22060,-20539,-19005,-17460,-15905,-14340,-12766,-11184,-9596,-8002,-6403,
-4801,-3195,-1588,20,1628,3236,4841,6444,8043,9636,11224,12806,14379,15944,17500,19044,20578,22099,
23606,25099,26578,28040,29485,30913,32322,33711,35080,36428,37754,39058,40338,41593,42824,44028,
45206,46357,47480,48575,49640,50675,51680,52653,53595,54504,55381,56224,57034,57809,58549,59254,
59923,60557,61153,61713,62236,62721,63168,63578,63949,64281,64575,64830,65046,65223,65360,65458,65516};

int SinTable2[256]={0,1608,3215,4821,6424,8022,9616,11204,12786,14359,15924,17480,19025,20558,22079,
23587,25081,26559,28021,29467,30895,32304,33694,35063,36411,37738,39041,40322,41577,42808,44013,
45192,46343,47466,48561,49626,50662,51667,52641,53583,54493,55370,56214,57024,57799,58540,59245,
59915,60549,61146,61706,62229,62715,63163,63573,63944,64277,64572,64827,65043,65221,65358,65457,
65516,65535,65516,65456,65357,65219,65042,64825,64569,64275,63941,63570,63159,62711,62225,61702,
61141,60544,59910,59240,58534,57793,57017,56207,55363,54486,53575,52633,51659,50653,49617,48552,
47457,46333,45182,44003,42798,41567,40311,39031,37727,36400,35052,33682,32292,30883,29455,28009,
26547,25068,23574,22067,20545,19012,17467,15911,14346,12772,11191,9603,8009,6410,4807,3202,1594,
-13,-1622,-3229,-4834,-6437,-8036,-9630,-11218,-12799,-14373,-15938,-17493,-19038,-20571,-22092,
-23600,-25093,-26571,-28034,-29479,-30907,-32316,-33705,-35075,-36423,-37749,-39052,-40332,-41588,-42818,-44023,-45201,-46352,-47476,-48570,-49635,-50671,-51675,-52649,-53591,-54501,-55377,-56221,-57030,-57806,-58546,-59251,-59921,-60554,-61151,-61711,
-62234,-62719,-63167,-63576,-63947,-64280,-64574,-64829,-65045,-65222,-65359,-65458,-65516,-65535,
-65515,-65456,-65356,-65218,-65040,-64823,-64567,-64272,-63938,-63566,-63156,-62707,-62221,-61697,
-61136,-60538,-59904,-59234,-58528,-57786,-57010,-56200,-55356,-54478,-53567,-52625,-51650,-50645,
-49609,-48543,-47447,-46324,-45172,-43993,-42788,-41556,-40300,-39020,-37716,-36389,-35040,-33670,
-32280,-30871,-29443,-27997,-26534,-25056,-23562,-22054,-20532,-18999,-17454,-15898,-14333,-12759,
-11178,-9589,-7995,-6397,-4794,-3188,-1581};



/***************************************************************************\
*  C4 C code                                                                *
\***************************************************************************/


short C4WFXVal;
short C4WFYVal;
short C4WFZVal;
short C4WFX2Val;
short C4WFY2Val;
short C4WFDist;
short C4WFScale;
double tanval;
double c4x,c4y,c4z;
double c4x2,c4y2,c4z2;

C4TransfWireFrame()
{
  c4x=(double)C4WFXVal;
  c4y=(double)C4WFYVal;
  c4z=(double)C4WFZVal-0x95;

  // Rotate X
  tanval=-(double)C4WFX2Val*3.14159265*2/128;
  c4y2=c4y*cos(tanval)-c4z*sin(tanval);
  c4z2=c4y*sin(tanval)+c4z*cos(tanval);

  // Rotate Y
  tanval=-(double)C4WFY2Val*3.14159265*2/128;
  c4x2=c4x*cos(tanval)+c4z2*sin(tanval);
  c4z=c4x*-sin(tanval)+c4z2*cos(tanval);

  // Rotate Z
  tanval=-(double)C4WFDist*3.14159265*2/128;
  c4x=c4x2*cos(tanval)-c4y2*sin(tanval);
  c4y=c4x2*sin(tanval)+c4y2*cos(tanval);

  // Scale
  C4WFXVal=c4x*(double)C4WFScale/(0x90*(c4z+0x95))*0x95;
  C4WFYVal=c4y*(double)C4WFScale/(0x90*(c4z+0x95))*0x95;
}

C4TransfWireFrame2()
{
  c4x=(double)C4WFXVal;
  c4y=(double)C4WFYVal;
  c4z=(double)C4WFZVal;

  // Rotate X
  tanval=-(double)C4WFX2Val*3.14159265*2/128;
  c4y2=c4y*cos(tanval)-c4z*sin(tanval);
  c4z2=c4y*sin(tanval)+c4z*cos(tanval);

  // Rotate Y
  tanval=-(double)C4WFY2Val*3.14159265*2/128;
  c4x2=c4x*cos(tanval)+c4z2*sin(tanval);
  c4z=c4x*-sin(tanval)+c4z2*cos(tanval);

  // Rotate Z
  tanval=-(double)C4WFDist*3.14159265*2/128;
  c4x=c4x2*cos(tanval)-c4y2*sin(tanval);
  c4y=c4x2*sin(tanval)+c4y2*cos(tanval);

  // Scale
  C4WFXVal=c4x*(double)C4WFScale/0x100;
  C4WFYVal=c4y*(double)C4WFScale/0x100;
}

C4CalcWireFrame()
{
  C4WFXVal=C4WFX2Val-C4WFXVal;
  C4WFYVal=C4WFY2Val-C4WFYVal;
  if (abs(C4WFXVal)>abs(C4WFYVal)){
    C4WFDist=abs(C4WFXVal)+1;
    C4WFYVal=256*(double)C4WFYVal/abs((double)C4WFXVal);
    if (C4WFXVal<0) C4WFXVal=-256;
    else C4WFXVal=256;
  }
  else
  if (C4WFYVal!=0) {
    C4WFDist=abs(C4WFYVal)+1;
    C4WFXVal=256*(double)C4WFXVal/abs((double)C4WFYVal);
    if (C4WFYVal<0) C4WFYVal=-256;
    else C4WFYVal=256;
  }
  else C4WFDist=0;
}

short C41FXVal;
short C41FYVal;
short C41FAngleRes;
short C41FDist;
short C41FDistVal;

C4Op1F()
{
  if (C41FXVal == 0) {
    if (C41FYVal>0) C41FAngleRes=0x80;
      else C41FAngleRes=0x180;
  }
  else {
    tanval = ((double)C41FYVal)/((double)C41FXVal);
    C41FAngleRes=atan(tanval)/(3.141592675*2)*512;
    C41FAngleRes=C41FAngleRes;
    if (C41FXVal<0) C41FAngleRes+=0x100;
    C41FAngleRes&=0x1FF;
  }
}

C4Op15()
{
  tanval=sqrt(((double)C41FYVal)*((double)C41FYVal)+((double)C41FXVal)*
    ((double)C41FXVal));
  C41FDist=tanval;
}

C4Op0D()
{
  tanval=sqrt(((double)C41FYVal)*((double)C41FYVal)+((double)C41FXVal)*
    ((double)C41FXVal));
  tanval=(double)C41FDistVal/tanval;
  C41FYVal=((double)C41FYVal*tanval)*0.99;
  C41FXVal=((double)C41FXVal*tanval)*0.98;
}


/***************************************************************************\
*  DSP1 code                                                                *
\***************************************************************************/


void InitDSP(void)
{
#ifdef DebugDSP1
   Start_Log();
#endif
}


short Op00Multiplicand;
short Op00Multiplier;
short Op00Result;

DSPOp00()
{
   Op00Result=Op00Multiplicand*Op00Multiplier/32768;
   #ifdef DebugDSP1
      Log_Message("OP00 MULT %d*%d/32768=%d",Op00Multiplicand,Op00Multiplier,Op00Result);
   #endif
}


short Op10Coefficient;
short Op10Exponent;
short Op10CoefficientR;
short Op10ExponentR;

// To fix...

DSPOp10()
{

#ifdef DebugDSP1
   Log_Message("OP10 NOT IMPLEMENTED");
#endif

   Op10ExponentR=-Op10Exponent;
   Op10CoefficientR=1/Op10Coefficient;
}


short Op04Angle;
unsigned short Op04Radius;
short Op04Sin;
short Op04Cos;

DSPOp04()
{
   Op04Sin=SinTable2[(Op04Angle/256)&255]*Op04Radius/65536;
   Op04Cos=CosTable2[(Op04Angle/256)&255]*Op04Radius/65536;
   #ifdef DebugDSP1
      Log_Message("OP04 Angle:%d Radius:%d",(Op04Angle/256)&255,Op04Radius);
      Log_Message("OP04 SIN:%d COS:%d",Op04Sin,Op04Cos);
   #endif
}


short Op08X;
short Op08Y;
short Op08Z;
int Op08Size;

DSPOp08Radius()
{
   Op08Size=(Op08X*Op08X+Op08Y*Op08Y+Op08Z*Op08Z)*2;

   #ifdef DebugDSP1
      Log_Message("OP08 %d,%d,%d",Op08X,Op08Y,Op08Z);
      Log_Message("OP08 ((Op08X^2)+(Op08Y^2)+(Op08X^2))*2=%d",Op08Size );
   #endif
}

short Op18X;
short Op18Y;
short Op18Z;
short Op18R;
short Op18Difference;

DSPOp18()
{
   Op18Difference=((Op18X*Op18X+Op18Y*Op18Y+Op18Z*Op18Z-Op18R*Op18R)*2)/65536;
   #ifdef DebugDSP1
      Log_Message("OP18 DIFF %d",Op18Difference);
   #endif
}

short Op28X;
short Op28Y;
short Op28Z;
short Op28R;

DSPOp28()
{
   Op28R=sqrt(abs(Op28X*Op28X+Op28Y*Op28Y+Op28Z*Op28Z));
   #ifdef DebugDSP1
      Log_Message("OP28 X:%d Y:%d Z:%d",Op18X,Op18Y,Op18Z);
      Log_Message("OP28 Vector Length %d",Op18R);
   #endif
}

unsigned short Op0CA;
short Op0CX1;
short Op0CY1;
short Op0CX2;
short Op0CY2;

DSPOp0C()
{
   Op0CX2=(Op0CX1*CosTable2[(Op0CA/256)&255]+Op0CY1*SinTable2[(Op0CA/256)&255])/65536;
   Op0CY2=(Op0CX1*-SinTable2[(Op0CA/256)&255]+Op0CY1*CosTable2[(Op0CA/256)&255])/65536;
   #ifdef DebugDSP1
      Log_Message("OP0C Angle:%d X:%d Y:%d CX:%d CY:%d",(Op0CA/256)&255,Op0CX1,Op0CY1,Op0CX2,Op0CY2);
   #endif
}

unsigned short Op1CAZ;
unsigned short Op1CAX;
unsigned short Op1CAY;
short Op1CX;
short Op1CY;
short Op1CZ;
short Op1CX1;
short Op1CY1;
short Op1CZ1;
short Op1CX2;
short Op1CY2;
short Op1CZ2;
short Op1CX3;
short Op1CY3;
short Op1CZ3;

DSPOp1C()
{
   // rotate around Y
   Op1CX1=(Op1CX*CosTable2[(Op1CAY/256)&255]+Op1CZ*SinTable2[(Op1CAY/256)&255])/65536;
   Op1CY1=Op1CY;
   Op1CZ1=(Op1CX*-SinTable2[(Op1CAY/256)&255]+Op1CZ*CosTable2[(Op1CAY/256)&255])/65536;
   // rotate around X
   Op1CX2=Op1CX1;
   Op1CY2=(Op1CY1*CosTable2[(Op1CAX/256)&255]+Op1CZ1*-SinTable2[(Op1CAX/256)&255])/65536;
   Op1CZ2=(Op1CY1*SinTable2[(Op1CAX/256)&255]+Op1CZ1*CosTable2[(Op1CAX/256)&255])/65536;
   // rotate around Z
   Op1CX3=(Op1CX2*CosTable2[(Op1CAZ/256)&255]+Op1CY2*-SinTable2[(Op1CAZ/256)&255])/65536;
   Op1CY3=(Op1CX2*SinTable2[(Op1CAZ/256)&255]+Op1CY2*CosTable2[(Op1CAZ/256)&255])/65536;
   Op1CZ3=Op1CZ2;

   #ifdef DebugDSP1
      Log_Message("OP1C Apply Matrix CX:%d CY:%d CZ",Op1CX3,Op1CY3,Op1CZ3);
   #endif
}




short Op02FX;
short Op02FY;
short Op02FZ;
short Op02LFE;
short Op02LES;
unsigned short Op02AAS;
unsigned short Op02AZS;
unsigned short Op02VOF;
unsigned short Op02VVA;

short Op02CX;
short Op02CY;
double Op02CXF;
double Op02CYF;
double ViewerX0;
double ViewerY0;
double ViewerZ0;
double ViewerX1;
double ViewerY1;
double ViewerZ1;
double ViewerX;
double ViewerY;
double ViewerZ;
int ViewerAX;
int ViewerAY;
int ViewerAZ;
double NumberOfSlope;
double ScreenX;
double ScreenY;
double ScreenZ;
double TopLeftScreenX;
double TopLeftScreenY;
double TopLeftScreenZ;
double BottomRightScreenX;
double BottomRightScreenY;
double BottomRightScreenZ;
double Ready;
double RasterLX;
double RasterLY;
double RasterLZ;
double ScreenLX1;
double ScreenLY1;
double ScreenLZ1;

DSPOp02()
{
   ViewerZ1=-cos(Op02AZS*6.2832/65536.0);
   ViewerX1=sin(Op02AZS*6.2832/65536.0)*sin(Op02AAS*6.2832/65536.0);
   ViewerY1=sin(Op02AZS*6.2832/65536.0)*cos(-Op02AAS*6.2832/65536.0);

   #ifdef debug02
   printf("\nViewerX1 : %f ViewerY1 : %f ViewerZ1 : %f\n",ViewerX1,ViewerY1,
                                                                   ViewerZ1);
   getch();
   #endif
   ViewerX=Op02FX-ViewerX1*Op02LFE;
   ViewerY=Op02FY-ViewerY1*Op02LFE;
   ViewerZ=Op02FZ-ViewerZ1*Op02LFE;

   ScreenX=Op02FX+ViewerX1*(Op02LES-Op02LFE);
   ScreenY=Op02FY+ViewerY1*(Op02LES-Op02LFE);
   ScreenZ=Op02FZ+ViewerZ1*(Op02LES-Op02LFE);

   #ifdef debug02
   printf("ViewerX : %f ViewerY : %f ViewerZ : %f\n",ViewerX,ViewerY,ViewerZ);
   printf("Op02FX : %d Op02FY : %d Op02FZ : %d\n",Op02FX,Op02FY,Op02FZ);
   printf("ScreenX : %f ScreenY : %f ScreenZ : %f\n",ScreenX,ScreenY,ScreenZ);
   getch();
   #endif
   if (ViewerZ1==0)ViewerZ1++;
   NumberOfSlope=ViewerZ/-ViewerZ1;

   Op02CX=ViewerX+ViewerX1*NumberOfSlope;
   Op02CY=ViewerY+ViewerY1*NumberOfSlope;

   Op02CXF=ViewerX+ViewerX1*NumberOfSlope;
   Op02CYF=ViewerY+ViewerY1*NumberOfSlope;

   Op02VOF=0x0000;
   if(Op02LFE==0x2200)Op02VVA=0xFECD;
   else Op02VVA=0xFFB2;
   #ifdef DebugDSP1
      Log_Message("OP02 FX:%d FY:%d FZ:%d LFE:%d LES:%d",Op02FX,Op02FY,Op02FZ,Op02LFE,Op02LES);
      Log_Message("     AAS:%d AZS:%d VOF:%d VVA:%d",Op02AAS,Op02AZS,Op02VOF,Op02VVA);
   #endif

}

short Op0AVS;
short Op0AA;
short Op0AB;
short Op0AC;
short Op0AD;

double RasterRX;
double RasterRY;
double RasterRZ;
double RasterLSlopeX;
double RasterLSlopeY;
double RasterLSlopeZ;
double RasterRSlopeX;
double RasterRSlopeY;
double RasterRSlopeZ;
double GroundLX;
double GroundLY;
double GroundRX;
double GroundRY;
double Distance;

DSPOp0A()
{
   if(Op0AVS==0)Op0AVS++;

   ScreenLZ1=-cos((Op02AZS-16384.0)*6.2832/65536.0);       // -16384.0
   ScreenLX1=sin((Op02AZS-16384.0)*6.2832/65536.0)*-sin(Op02AAS*6.2832/65536.0);
   ScreenLY1=-sin((Op02AZS-16384.0)*6.2832/65536.0)*-cos(-Op02AAS*6.2832/65536.0);

   RasterRX=RasterLX=ScreenX+(Op0AVS)*ScreenLX1;
   RasterRY=RasterLY=ScreenY+(Op0AVS)*ScreenLY1;
   RasterRZ=RasterLZ=ScreenZ+(Op0AVS)*ScreenLZ1;

   ScreenLX1=sin((Op02AAS+16384.0)*6.2832/65536);
   ScreenLY1=cos(-(Op02AAS+16384.0)*6.2832/65536);

   RasterLX=RasterLX-128*ScreenLX1;
   RasterLY=RasterLY-128*ScreenLY1;

   RasterRX=RasterRX+128*ScreenLX1;
   RasterRY=RasterRY+128*ScreenLY1;

   Distance=Op02LFE;
   if(Distance==0)Distance=1;

   RasterLSlopeX=(RasterLX-ViewerX)/Distance;
   RasterLSlopeY=(RasterLY-ViewerY)/Distance;
   RasterLSlopeZ=(RasterLZ-ViewerZ)/Distance;

   RasterRSlopeX=(RasterRX-ViewerX)/Distance;
   RasterRSlopeY=(RasterRY-ViewerY)/Distance;
   RasterRSlopeZ=(RasterRZ-ViewerZ)/Distance;

   if(RasterLSlopeZ==0) RasterLSlopeZ++; // divide by 0

   NumberOfSlope=ViewerZ/-RasterLSlopeZ;

   GroundLX=ViewerX+RasterLSlopeX*NumberOfSlope;
   GroundLY=ViewerY+RasterLSlopeY*NumberOfSlope;

   if(RasterRSlopeZ==0) RasterRSlopeZ++; // divide by 0

   NumberOfSlope=ViewerZ/-RasterRSlopeZ;

   GroundRX=ViewerX+RasterRSlopeX*NumberOfSlope;
   GroundRY=ViewerY+RasterRSlopeY*NumberOfSlope;


   if(Op02LES==0)Op02LES=1;

   Op0AA=(GroundRX-GroundLX);
   Op0AB=(GroundRY-GroundLY);  //0x300/Op02LES*2;

   if(Op0AVS!=0)
   {
      Op0AC=(((Op02CXF)-((GroundRX+GroundLX)/2)))/Op0AVS;
      Op0AD=(((Op02CYF)-((GroundRY+GroundLY)/2)))/Op0AVS*256;
   }
   else
   {
      Op0AC=0;
      Op0AD=0;
   }
   Op0AVS+=1;
}

short Op06X;
short Op06Y;
short Op06Z;
short Op06H;
short Op06V;
unsigned short Op06S;

double ObjPX;
double ObjPY;
double ObjPZ;
double ObjPX1;
double ObjPY1;
double ObjPZ1;
double ObjPX2;
double ObjPY2;
double ObjPZ2;
double DivideOp06;
int Temp;

DSPOp06()
{

   ObjPX=Op06X-Op02CXF;
   ObjPY=Op06Y-Op02CYF;
   ObjPZ=Op06Z;

   // rotate around Z
   ObjPX1=(ObjPX*cos((-Op02AAS+32768)/65536.0*6.2832)+ObjPY*-sin((-Op02AAS+32768)/65536.0*6.2832));
   ObjPY1=(ObjPX*sin((-Op02AAS+32768)/65536.0*6.2832)+ObjPY*cos((-Op02AAS+32768)/65536.0*6.2832));
   ObjPZ1=ObjPZ;

   // rotate around X
   ObjPX2=ObjPX1;
   ObjPY2=(ObjPY1*cos((-Op02AZS)/65536.0*6.2832)+ObjPZ1*-sin((-Op02AZS)/65536.0*6.2832));
   ObjPZ2=(ObjPY1*sin((-Op02AZS)/65536.0*6.2832)+ObjPZ1*cos((-Op02AZS)/65536.0*6.2832));

   #ifdef debug06
   printf("ObjPX2: %f ObjPY2: %f ObjPZ2: %f\n",ObjPX2,ObjPY2,ObjPZ2);
   #endif

   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPZ2<0)
   {
      Op06H=-ObjPX2*Op02LES/-(ObjPZ2); //-ObjPX2*256/-ObjPZ2;
      Op06V=-ObjPY2*Op02LES/-(ObjPZ2); //-ObjPY2*256/-ObjPZ2;
      Op06S=256*Op02LES/-ObjPZ2;
   }
   else
   {
      Op06H=0;
      Op06V=14*16;
   }

   #ifdef DebugDSP1
      Log_Message("OP06 X:%d Y:%d Z:%d",Op06X,Op06Y,Op06Z);
      Log_Message("OP06 H:%d V:%d S:%d",Op06H,Op06V,Op06S);
   #endif
}



double matrix[4][4];
double smat[4][4];
double tmat[4][4];
double xmat[4][4];
double ymat[4][4];
double zmat[4][4];
double matrix0[4][4];
double matrix1[4][4];
double matrix2[4][4];
double matrixI0[4][4];
double matrixI1[4][4];
double matrixI2[4][4];

void InitMatrix() 
{
   matrix[0][0]=1; matrix[0][1]=0; matrix[0][2]=0; matrix[0][3]=0;
   matrix[1][0]=0; matrix[1][1]=1; matrix[1][2]=0; matrix[1][3]=0;
   matrix[2][0]=0; matrix[2][1]=0; matrix[2][2]=1; matrix[2][3]=0;
   matrix[3][0]=0; matrix[3][1]=0; matrix[3][2]=0; matrix[3][3]=1;
}


void MultMatrix(double result[4][4],double mat1[4][4],double mat2[4][4])
{
   result[0][0]=0;
   result[0][0]+=(mat1[0][0]*mat2[0][0]+mat1[0][1]*mat2[1][0]+mat1[0][2]*mat2[2][0]+mat1[0][3]*mat2[3][0]);
   result[0][1]=0;
   result[0][1]+=(mat1[0][0]*mat2[0][1]+mat1[0][1]*mat2[1][1]+mat1[0][2]*mat2[2][1]+mat1[0][3]*mat2[3][1]);
   result[0][2]=0;
   result[0][2]+=(mat1[0][0]*mat2[0][2]+mat1[0][1]*mat2[1][2]+mat1[0][2]*mat2[2][2]+mat1[0][3]*mat2[3][2]);
   result[0][3]=0;
   result[0][3]+=(mat1[0][0]*mat2[0][3]+mat1[0][1]*mat2[1][3]+mat1[0][2]*mat2[2][3]+mat1[0][3]*mat2[3][3]);

   result[1][0]=0;
   result[1][0]+=(mat1[1][0]*mat2[0][0]+mat1[1][1]*mat2[1][0]+mat1[1][2]*mat2[2][0]+mat1[1][3]*mat2[3][0]);
   result[1][1]=0;
   result[1][1]+=(mat1[1][0]*mat2[0][1]+mat1[1][1]*mat2[1][1]+mat1[1][2]*mat2[2][1]+mat1[1][3]*mat2[3][1]);
   result[1][2]=0;
   result[1][2]+=(mat1[1][0]*mat2[0][2]+mat1[1][1]*mat2[1][2]+mat1[1][2]*mat2[2][2]+mat1[1][3]*mat2[3][2]);
   result[1][3]=0;
   result[1][3]+=(mat1[1][0]*mat2[0][3]+mat1[1][1]*mat2[1][3]+mat1[1][2]*mat2[2][3]+mat1[1][3]*mat2[3][3]);
   
   result[2][0]=0;
   result[2][0]+=(mat1[2][0]*mat2[0][0]+mat1[2][1]*mat2[1][0]+mat1[2][2]*mat2[2][0]+mat1[2][3]*mat2[3][0]);
   result[2][1]=0;
   result[2][1]+=(mat1[2][0]*mat2[0][1]+mat1[2][1]*mat2[1][1]+mat1[2][2]*mat2[2][1]+mat1[2][3]*mat2[3][1]);
   result[2][2]=0;
   result[2][2]+=(mat1[2][0]*mat2[0][2]+mat1[2][1]*mat2[1][2]+mat1[2][2]*mat2[2][2]+mat1[2][3]*mat2[3][2]);
   result[2][3]=0;                    
   result[2][3]+=(mat1[2][0]*mat2[0][3]+mat1[2][1]*mat2[1][3]+mat1[2][2]*mat2[2][3]+mat1[2][3]*mat2[3][3]);

   result[3][0]=0;
   result[3][0]+=(mat1[3][0]*mat2[0][0]+mat1[3][1]*mat2[1][0]+mat1[3][2]*mat2[2][0]+mat1[3][3]*mat2[3][0]);
   result[3][1]=0;
   result[3][1]+=(mat1[3][0]*mat2[0][1]+mat1[3][1]*mat2[1][1]+mat1[3][2]*mat2[2][1]+mat1[3][3]*mat2[3][1]);
   result[3][2]=0;
   result[3][2]+=(mat1[3][0]*mat2[0][2]+mat1[3][1]*mat2[1][2]+mat1[3][2]*mat2[2][2]+mat1[3][3]*mat2[3][2]);
   result[3][3]=0;
   result[3][3]+=(mat1[3][0]*mat2[0][3]+mat1[3][1]*mat2[1][3]+mat1[3][2]*mat2[2][3]+mat1[3][3]*mat2[3][3]);
}

void CopyMatrix(double dest[4][4],double source[4][4])
{
   dest[0][0]=source[0][0];
   dest[0][1]=source[0][1];
   dest[0][2]=source[0][2];
   dest[0][3]=source[0][3];
   dest[1][0]=source[1][0];
   dest[1][1]=source[1][1];
   dest[1][2]=source[1][2];
   dest[1][3]=source[1][3];
   dest[2][0]=source[2][0];
   dest[2][1]=source[2][1];
   dest[2][2]=source[2][2];
   dest[2][3]=source[2][3];
   dest[3][0]=source[3][0];
   dest[3][1]=source[3][1];
   dest[3][2]=source[3][2];
   dest[3][3]=source[3][3];
}

void scale(double sf)
{
   double mat[4][4];
   smat[0][0]=sf; smat[0][1]=0; smat[0][2]=0; smat[0][3]=0;
   smat[1][0]=0; smat[1][1]=sf; smat[1][2]=0; smat[1][3]=0;
   smat[2][0]=0; smat[2][1]=0; smat[2][2]=sf; smat[2][3]=0;
   smat[3][0]=0; smat[3][1]=0; smat[3][2]=0; smat[3][3]=1;
   MultMatrix(mat,smat,matrix);
   CopyMatrix(matrix,mat);
}

void translate(double xt,double yt,double zt)
{
   double mat[4][4];
   tmat[0][0]=1; tmat[0][1]=0; tmat[0][2]=0; tmat[0][3]=0;
   tmat[1][0]=0; tmat[1][1]=1; tmat[1][2]=0; tmat[1][3]=0;
   tmat[2][0]=0; tmat[2][1]=0; tmat[2][2]=1; tmat[2][3]=0;
   tmat[3][0]=xt; tmat[3][1]=yt; tmat[3][2]=zt; tmat[3][3]=1;
   MultMatrix(mat,matrix,tmat);
   CopyMatrix(matrix,mat);
}

void rotate(double ax,double ay,double az)
{
   double mat1[4][4];
   double mat2[4][4];
   xmat[0][0]=1; xmat[0][1]=0; xmat[0][2]=0; xmat[0][3]=0;
   xmat[1][0]=0; xmat[1][1]=cos(ax); xmat[1][2]=sin(ax); xmat[1][3]=0;
   xmat[2][0]=0; xmat[2][1]=-(sin(ax)); xmat[2][2]=cos(ax); xmat[2][3]=0;
   xmat[3][0]=0; xmat[3][1]=0; xmat[3][2]=0; xmat[3][3]=1;
   MultMatrix(mat1,xmat,matrix);
   ymat[0][0]=cos(ay); ymat[0][1]=0; ymat[0][2]=-(sin(ay)); ymat[0][3]=0;
   ymat[1][0]=0; ymat[1][1]=1; ymat[1][2]=0; ymat[1][3]=0;
   ymat[2][0]=sin(ay); ymat[2][1]=0; ymat[2][2]=cos(ay); ymat[2][3]=0;
   ymat[3][0]=0; ymat[3][1]=0; ymat[3][2]=0; ymat[3][3]=1;
   MultMatrix(mat2,ymat,mat1);
   zmat[0][0]=cos(az); zmat[0][1]=sin(az); zmat[0][2]=0; zmat[0][3]=0;
   zmat[1][0]=-(sin(az)); zmat[1][1]=cos(az); zmat[1][2]=0; zmat[1][3]=0;
   zmat[2][0]=0; zmat[2][1]=0; zmat[2][2]=1; zmat[2][3]=0;
   zmat[3][0]=0; zmat[3][1]=0; zmat[3][2]=0; zmat[3][3]=1;
   MultMatrix(matrix,zmat,mat2);
}

short Op01m;
short Op01Zr;
short Op01Xr;
short Op01Yr;

DSPOp01()
{
   InitMatrix();
   rotate(Op01Xr/65536.0*6.2832,Op01Yr/65536.0*6.2832,Op01Zr/65536.0*6.2832);
   CopyMatrix(matrix0,matrix);
   InitMatrix();
   rotate(0,0,Op01Zr/65536.0*6.2832);
   rotate(Op01Xr/65536.0*6.2832,0,0);
   rotate(0,Op01Yr/65536.0*6.2832,0);
   CopyMatrix(matrixI0,matrix);
   #ifdef DebugDSP1
      Log_Message("OP01");
   #endif
}

DSPOp11()
{
   InitMatrix();
   rotate(Op01Xr/65536.0*6.2832,Op01Yr/65536.0*6.2832,Op01Zr/65536.0*6.2832);
   CopyMatrix(matrix1,matrix);
   InitMatrix();
   rotate(0,0,Op01Zr/65536.0*6.2832);
   rotate(Op01Xr/65536.0*6.2832,0,0);
   rotate(0,Op01Yr/65536.0*6.2832,0);
   CopyMatrix(matrixI1,matrix);
   #ifdef DebugDSP1
      Log_Message("OP11");
   #endif

}

DSPOp21()
{
   InitMatrix();
   rotate(Op01Xr/65536.0*6.2832,Op01Yr/65536.0*6.2832,Op01Zr/65536.0*6.2832);
   CopyMatrix(matrix2,matrix);
   InitMatrix();
   rotate(0,0,Op01Zr/65536.0*6.2832);
   rotate(Op01Xr/65536.0*6.2832,0,0);
   rotate(0,Op01Yr/65536.0*6.2832,0);
   CopyMatrix(matrixI2,matrix);
   #ifdef DebugDSP1
      Log_Message("OP21");
   #endif
}



short Op0DX;
short Op0DY;
short Op0DZ;
short Op0DF;
short Op0DL;
short Op0DU;

DSPOp0D()
{
   Op0DF=(double)(Op0DX*matrixI0[0][0]+Op0DY*matrixI0[1][0]+Op0DZ*matrixI0[2][0]+matrixI0[3][0]);
   Op0DL=(double)(Op0DX*matrixI0[0][1]+Op0DY*matrixI0[1][1]+Op0DZ*matrixI0[2][1]+matrixI0[3][1]);
   Op0DU=(double)(Op0DX*matrixI0[0][2]+Op0DY*matrixI0[1][2]+Op0DZ*matrixI0[2][2]+matrixI0[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP0D");
   #endif
}

DSPOp1D()
{
   Op0DF=(double)(Op0DX*matrixI1[0][0]+Op0DY*matrixI1[1][0]+Op0DZ*matrixI1[2][0]+matrixI1[3][0]);
   Op0DL=(double)(Op0DX*matrixI1[0][1]+Op0DY*matrixI1[1][1]+Op0DZ*matrixI1[2][1]+matrixI1[3][1]);
   Op0DU=(double)(Op0DX*matrixI1[0][2]+Op0DY*matrixI1[1][2]+Op0DZ*matrixI1[2][2]+matrixI1[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP1D");
   #endif
}

DSPOp2D()
{
   Op0DF=(double)(Op0DX*matrixI2[0][0]+Op0DY*matrixI2[1][0]+Op0DZ*matrixI2[2][0]+matrixI2[3][0]);
   Op0DL=(double)(Op0DX*matrixI2[0][1]+Op0DY*matrixI2[1][1]+Op0DZ*matrixI2[2][1]+matrixI2[3][1]);
   Op0DU=(double)(Op0DX*matrixI2[0][2]+Op0DY*matrixI2[1][2]+Op0DZ*matrixI2[2][2]+matrixI2[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP2D");
   #endif
}

short Op03F;
short Op03L;
short Op03U;
short Op03X;
short Op03Y;
short Op03Z;

DSPOp03()
{
   Op03X=(double)(Op03F*matrix0[0][0]+Op03L*matrix0[1][0]+Op03U*matrix0[2][0]+matrix0[3][0]);
   Op03Y=(double)(Op03F*matrix0[0][1]+Op03L*matrix0[1][1]+Op03U*matrix0[2][1]+matrix0[3][1]);
   Op03Z=(double)(Op03F*matrix0[0][2]+Op03L*matrix0[1][2]+Op03U*matrix0[2][2]+matrix0[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP03");
   #endif
}

DSPOp13()
{
   Op03X=(double)(Op03F*matrix1[0][0]+Op03L*matrix1[1][0]+Op03U*matrix1[2][0]+matrix1[3][0]);
   Op03Y=(double)(Op03F*matrix1[0][1]+Op03L*matrix1[1][1]+Op03U*matrix1[2][1]+matrix1[3][1]);
   Op03Z=(double)(Op03F*matrix1[0][2]+Op03L*matrix1[1][2]+Op03U*matrix1[2][2]+matrix1[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP13");
   #endif
}

DSPOp23()
{
   Op03X=(double)(Op03F*matrix2[0][0]+Op03L*matrix2[1][0]+Op03U*matrix2[2][0]+matrix2[3][0]);
   Op03Y=(double)(Op03F*matrix2[0][1]+Op03L*matrix2[1][1]+Op03U*matrix2[2][1]+matrix2[3][1]);
   Op03Z=(double)(Op03F*matrix2[0][2]+Op03L*matrix2[1][2]+Op03U*matrix2[2][2]+matrix2[3][2]);
   #ifdef DebugDSP1
      Log_Message("OP23");
   #endif
}

short Op14Zr;
short Op14Xr;
short Op14Yr;
short Op14U;
short Op14F;
short Op14L;
short Op14Zrr;
short Op14Xrr;
short Op14Yrr;

double Op14Temp;
DSPOp14()
{
   Op14Temp=(Op14Zr*6.2832/65536.0)+(1/cos(Op14Xr*6.2832/65536.0))*((Op14U*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0)-(Op14F*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0));
   Op14Zrr=Op14Temp*65536.0/6.2832;
   Op14Temp=(Op14Xr*6.2832/65536.0)+((Op14U*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0));
   Op14Xrr=Op14Temp*65536.0/6.2832;
   Op14Temp=(Op14Yr*6.2832/65536.0)-tan(Op14Xr*6.2832/65536.0)*((Op14U*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0))+(Op14L*6.2832/65536.0);
   Op14Yrr=Op14Temp*65536.0/6.2832;
   #ifdef DebugDSP1
      Log_Message("OP14 X:%d Y%d Z:%D U:%d F:%d L:%d",Op14Xr,Op14Yr,Op14Zr,Op14U,Op14F,Op14L);
      Log_Message("OP14 X:%d Y%d Z:%D",Op14Xrr,Op14Yrr,Op14Zrr);
   #endif
}

short Op0EH;
short Op0EV;
short Op0EX;
short Op0EY;
DSPOp0E()
{

   // screen Directions UP
   ScreenLZ1=-cos((Op02AZS-16384.0)*6.2832/65536.0);       // -16384.0
   ScreenLX1=sin((Op02AZS-16384.0)*6.2832/65536.0)*-sin(Op02AAS*6.2832/65536.0);
   ScreenLY1=-sin((Op02AZS-16384.0)*6.2832/65536.0)*-cos(-Op02AAS*6.2832/65536.0);

   RasterLX=ScreenX+(Op0EV)*ScreenLX1;
   RasterLY=ScreenY+(Op0EV)*ScreenLY1;
   RasterLZ=ScreenZ+(Op0EV)*ScreenLZ1;

   // screen direction right
   ScreenLX1=sin((Op02AAS+16384.0)*6.2832/65536);
   ScreenLY1=cos(-(Op02AAS+16384.0)*6.2832/65536);

   RasterLX=RasterLX+Op0EH*ScreenLX1;
   RasterLY=RasterLY+Op0EH*ScreenLY1;

   Distance=Op02LFE;

   if(Distance==0)Distance=1;

   RasterLSlopeX=(RasterLX-ViewerX)/Distance;
   RasterLSlopeY=(RasterLY-ViewerY)/Distance;
   RasterLSlopeZ=(RasterLZ-ViewerZ)/Distance;

   if(RasterLSlopeZ==0)RasterLSlopeZ++;

   NumberOfSlope=ViewerZ/-RasterLSlopeZ;

   GroundLX=ViewerX+RasterLSlopeX*NumberOfSlope;
   GroundLY=ViewerY+RasterLSlopeY*NumberOfSlope;

   Op0EX=GroundLX;
   Op0EY=GroundLY;

   #ifdef DebugDSP1
      Log_Message("OP0E COORDINATE H:%d V:%d",Op0EH,Op0EV);
      Log_Message("                X:%d Y:%d",Op0EX,Op0EY);
   #endif
}
