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
#include <string.h>

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
   fflush (LogFile);
}

void Start_Log (void)
{
	char LogFileName[255];
//  [4/15/2001]	char *p;

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
  C4WFXVal=(short)(c4x*C4WFScale/(0x90*(c4z+0x95))*0x95);
  C4WFYVal=(short)(c4y*C4WFScale/(0x90*(c4z+0x95))*0x95);
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
  C4WFXVal=(short)(c4x*C4WFScale/0x100);
  C4WFYVal=(short)(c4y*C4WFScale/0x100);
}

C4CalcWireFrame()
{
  C4WFXVal=C4WFX2Val-C4WFXVal;
  C4WFYVal=C4WFY2Val-C4WFYVal;
  if (abs(C4WFXVal)>abs(C4WFYVal)){
    C4WFDist=abs(C4WFXVal)+1;
    C4WFYVal=(256*(long)C4WFYVal)/abs(C4WFXVal);
    if (C4WFXVal<0) C4WFXVal=-256;
    else C4WFXVal=256;
  }
  else
  if (C4WFYVal!=0) {
    C4WFDist=abs(C4WFYVal)+1;
    C4WFXVal=(256*(long)C4WFXVal)/abs(C4WFYVal);
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
    C41FAngleRes=(short)(atan(tanval)/(3.141592675*2)*512);
    C41FAngleRes=C41FAngleRes;
    if (C41FXVal<0) C41FAngleRes+=0x100;
    C41FAngleRes&=0x1FF;
  }
}

C4Op15()
{
  tanval=sqrt(((double)C41FYVal)*((double)C41FYVal)+((double)C41FXVal)*
    ((double)C41FXVal));
  C41FDist=(short)tanval;
}

C4Op0D()
{
  tanval=sqrt(((double)C41FYVal)*((double)C41FYVal)+((double)C41FXVal)*
    ((double)C41FXVal));
  tanval=(double)C41FDistVal/tanval;
  C41FYVal=(short)(((double)C41FYVal*tanval)*0.99);
  C41FXVal=(short)(((double)C41FXVal*tanval)*0.98);
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

signed short Op10Coefficient;
signed short Op10Exponent;
signed short Op10CoefficientR;
signed short Op10ExponentR;
float Op10Temp;

DSPOp10()
{
        Op10ExponentR=-Op10Exponent;
        Op10Temp = Op10Coefficient / 32768.0;
	if (Op10Temp == 0) {
		Log_Message("OP10 : DIVISION BY ZERO :(");
		Op10CoefficientR = 0;
	} else
		Op10Temp = 1/Op10Temp;	
        if (Op10Temp > 0) 
                while (Op10Temp>=1.0) {
                        Op10Temp=Op10Temp/2.0;
                        Op10ExponentR++;
                }
        else
                while (Op10Temp<-1.0) {
                        Op10Temp=Op10Temp/2.0;
                        Op10ExponentR++;
                }
        Op10CoefficientR = Op10Temp*32768;
	#ifdef DebugDSP1
        Log_Message("OP10 INV %d*2^%d = %d*2^%d", Op10Coefficient, Op10Exponent, Op10CoefficientR, Op10ExponentR);
	#endif
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


/*DSPOp08Radius()
{
   Op08Size=(Op08X*Op08X+Op08Y*Op08Y+Op08Z*Op08Z)*2;

   #ifdef DebugDSP1
      Log_Message("OP08 %d,%d,%d",Op08X,Op08Y,Op08Z);
      Log_Message("OP08 ((Op08X^2)+(Op08Y^2)+(Op08X^2))*2=%d",Op08Size );
   #endif
}*/

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
int    ReversedLES;
short Op02LESb;
double NAzsB,NAasB;
double ViewerXc;
double ViewerYc;
double ViewerZc;
double CenterX,CenterY;
short Op02CYSup,Op02CXSup;
double CXdistance;

#define VofAngle 0x3880

short TValDebug,TValDebug2;
short ScrDispl;

void DSPOp02()
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

   Op02CX=(short)(Op02CXF=ViewerX+ViewerX1*NumberOfSlope);
   Op02CY=(short)(Op02CYF=ViewerY+ViewerY1*NumberOfSlope);

   ViewerXc=ViewerX;//-Op02FX);
   ViewerYc=ViewerY;//-Op02FY);
   ViewerZc=ViewerZ;//-Op02FZ);

   Op02VOF=0x0000;
   ReversedLES=0;
   Op02LESb=Op02LES;
   if ((Op02LES>=VofAngle+16384.0) && (Op02LES<VofAngle+32768.0)) {
     ReversedLES=1;
     Op02LESb=VofAngle+0x4000-(Op02LES-(VofAngle+0x4000));
   }
   Op02VVA = (short)(Op02LESb * tan((Op02AZS-0x4000)*6.2832/65536.0));
   if ((Op02LESb>=VofAngle) && (Op02LESb<=VofAngle+0x4000)) {
      Op02VOF= (short)(Op02LESb * tan((Op02AZS-0x4000-VofAngle)*6.2832/65536.0));
      Op02VVA-=Op02VOF;
   }
   if (ReversedLES){
     Op02VOF=-Op02VOF;
   }

   NAzsB = (Op02AZS-0x4000)*6.2832/65536.0;
   NAasB = Op02AAS*6.2832/65536.0;

   if (tan(NAzsB)==0) return;

   ScrDispl=0;
   if (NAzsB>-0.15) {NAzsB=-0.15;ScrDispl=Op02VVA-0xFFDA;}

   CXdistance=1/tan(NAzsB);

   CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   if (CenterX<-32768) CenterX = -32768; if (CenterX>32767) CenterX=32767;
   Op02CX = (short)CenterX;
   CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   if (CenterY<-32768) CenterY = -32768; if (CenterY>32767) CenterY=32767;
   Op02CY = (short)CenterY;

   TValDebug = (NAzsB*65536/6.28);
   TValDebug2 = ScrDispl;

//   if (Op02CY < 0) {Op02CYSup = Op02CY/256; Op02CY = 0;}
//   if (Op02CX < 0) {Op02CXSup = Op02CX/256; Op02CX = 0;}

//  [4/15/2001]   (ViewerX+ViewerX1*NumberOfSlope);
//  [4/15/2001]   (ViewerY+ViewerY1*NumberOfSlope);

//   if(Op02LFE==0x2200)Op02VVA=0xFECD;
//   else Op02VVA=0xFFB2;


   #ifdef DebugDSP1
      Log_Message("OP02 FX:%d FY:%d FZ:%d LFE:%d LES:%d",Op02FX,Op02FY,Op02FZ,Op02LFE,Op02LES);
      Log_Message("     AAS:%d AZS:%d VOF:%d VVA:%d",Op02AAS,Op02AZS,Op02VOF,Op02VVA);
      Log_Message("     VX:%d VY:%d VZ:%d",(short)ViewerX,(short)ViewerY,(short)ViewerZ);
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

double NAzs,NAas;
double RVPos,RHPos,RXRes,RYRes;


void GetRXYPos(){
   double scalar;

   if (Op02LES==0) return;


   NAzs = NAzsB - atan((RVPos) / (double)Op02LES);
   NAas = NAasB;// + atan(RHPos) / (double)Op02LES);

   if (cos(NAzs)==0) return;
   if (tan(NAzs)==0) return;

   RXRes = (-sin(NAas)*ViewerZc/(tan(NAzs))+ViewerXc);
   RYRes = (cos(NAas)*ViewerZc/(tan(NAzs))+ViewerYc);
   scalar = ((ViewerZc/sin(NAzs))/(double)Op02LES);
   RXRes += scalar*-sin(NAas+3.14159/2)*RHPos;
   RYRes += scalar*cos(NAas+3.14159/2)*RHPos;
}

void DSPOp0A()
{
  double x2,y2,x3,y3,x4,y4,m,ypos;


   if(Op0AVS==0) {Op0AVS++; return;}
   ypos=Op0AVS-ScrDispl;
   // CenterX,CenterX = Center (x1,y1)
   // Get (0,Vs) coords (x2,y2)
   RVPos = ypos; RHPos = 0;
   GetRXYPos(); x2 = RXRes; y2 = RYRes;
   // Get (-128,Vs) coords (x3,y3)
   RVPos = ypos; RHPos = -128;
   GetRXYPos(); x3 = RXRes; y3 = RYRes;
   // Get (127,Vs) coords (x4,y4)
   RVPos = ypos; RHPos = 127;
   GetRXYPos(); x4 = RXRes; y4 = RYRes;

   // A = (x4-x3)/256
   m = (x4-x3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
   Op0AA = (short)(m);
   // C = (y4-y3)/256
   m = (y4-y3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
   Op0AC = (short)(m);
   if (ypos==0){
     Op0AB = 0;
     Op0AD = 0;
   }
   else {
     // B = (x2-x1)/Vs
     m = (x2-CenterX)/ypos*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
     Op0AB = (short)(m);
     // D = (y2-y1)/Vs
     m = (y2-CenterY)/ypos*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
     Op0AD = (short)(m);
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

/*   ObjPX=(Op06X-CenterX);
   ObjPY=-(Op06Y-CenterY);
   ObjPZ=-(Op06Z-0);

   // rotate around Z
   tanval = (Op02AAS)/65536.0*6.2832;
   ObjPX1=(ObjPX*cos(tanval)+ObjPY*-sin(tanval));
   ObjPY1=(ObjPX*sin(tanval)+ObjPY*cos(tanval));
   ObjPZ1=ObjPZ;

   // rotate around Y
//   ObjPX2=ObjPX1;
//   ObjPY2=(ObjPY1*cos((-Op02AZS)/65536.0*6.2832)+ObjPZ1*-sin((-Op02AZS)/65536.0*6.2832));
//   ObjPZ2=(ObjPY1*sin((-Op02AZS)/65536.0*6.2832)+ObjPZ1*cos((-Op02AZS)/65536.0*6.2832));
   // Rotate around X
   tanval=((Op02AZS-16384.0))/65536.0*6.2832;
   ObjPX2=ObjPX1;
   ObjPY2=ObjPY1*cos(tanval)+ObjPZ1*-sin(tanval);
   ObjPZ2=ObjPY1*sin(tanval)+ObjPZ1*cos(tanval);

//   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPY2>0)
   {
      Op06H=(short)(ObjPX2*Op02LES/(ObjPY2)); //-ObjPX2*256/-ObjPZ2;
      Op06V=(short)(ObjPZ2*Op02LES/(ObjPY2)); //-ObjPY2*256/-ObjPZ2;
      Op06S=(unsigned short)(256*(double)Op02LES/ObjPY2);
   }
   else
   {
      Op06H=0;
      Op06V=14*16;
   }*/


   ObjPX=Op06X-Op02FX;
   ObjPY=Op06Y-Op02FY;
   ObjPZ=Op06Z-Op02FZ;

   // rotate around Z
   tanval = (-Op02AAS+32768)/65536.0*6.2832;
   ObjPX1=(ObjPX*cos(tanval)+ObjPY*-sin(tanval));
   ObjPY1=(ObjPX*sin(tanval)+ObjPY*cos(tanval));
   ObjPZ1=ObjPZ;

   // rotate around X
   tanval = (-Op02AZS)/65536.0*6.2832;
   ObjPX2=ObjPX1;
   ObjPY2=(ObjPY1*cos(tanval)+ObjPZ1*-sin(tanval));
   ObjPZ2=(ObjPY1*sin(tanval)+ObjPZ1*cos(tanval));

   #ifdef debug06
   printf("ObjPX2: %f ObjPY2: %f ObjPZ2: %f\n",ObjPX2,ObjPY2,ObjPZ2);
   #endif

   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPZ2<0)
   {
      Op06H=(short)(-ObjPX2*Op02LES/-(ObjPZ2)); //-ObjPX2*256/-ObjPZ2;
      Op06V=(short)(-ObjPY2*Op02LES/-(ObjPZ2)); //-ObjPY2*256/-ObjPZ2;
      Op06S=(unsigned short)(256*(double)Op02LES/-ObjPZ2);
   }
   else
   {
      Op06H=0;
      Op06V=14*16;
      Op06S=0xFFFF;
   }


   #ifdef DebugDSP1
      Log_Message("OP06 X:%d Y:%d Z:%d",Op06X,Op06Y,Op06Z);
      Log_Message("OP06 H:%d V:%d S:%d",Op06H,Op06V,Op06S);
   #endif
}

double matrixB[3][3];
double matrixB2[3][3];
double matrixB3[3][3];

double matrixA[3][3];
double matrixA2[3][3];
double matrixA3[3][3];

void MultMatrixB(double result[3][3],double mat1[3][3],double mat2[3][3])
{
   result[0][0]=0;
   result[0][0]+=(mat1[0][0]*mat2[0][0]+mat1[0][1]*mat2[1][0]+mat1[0][2]*mat2[2][0]);
   result[0][1]=0;
   result[0][1]+=(mat1[0][0]*mat2[0][1]+mat1[0][1]*mat2[1][1]+mat1[0][2]*mat2[2][1]);
   result[0][2]=0;
   result[0][2]+=(mat1[0][0]*mat2[0][2]+mat1[0][1]*mat2[1][2]+mat1[0][2]*mat2[2][2]);

   result[1][0]=0;
   result[1][0]+=(mat1[1][0]*mat2[0][0]+mat1[1][1]*mat2[1][0]+mat1[1][2]*mat2[2][0]);
   result[1][1]=0;
   result[1][1]+=(mat1[1][0]*mat2[0][1]+mat1[1][1]*mat2[1][1]+mat1[1][2]*mat2[2][1]);
   result[1][2]=0;
   result[1][2]+=(mat1[1][0]*mat2[0][2]+mat1[1][1]*mat2[1][2]+mat1[1][2]*mat2[2][2]);
   
   result[2][0]=0;
   result[2][0]+=(mat1[2][0]*mat2[0][0]+mat1[2][1]*mat2[1][0]+mat1[2][2]*mat2[2][0]);
   result[2][1]=0;
   result[2][1]+=(mat1[2][0]*mat2[0][1]+mat1[2][1]*mat2[1][1]+mat1[2][2]*mat2[2][1]);
   result[2][2]=0;
   result[2][2]+=(mat1[2][0]*mat2[0][2]+mat1[2][1]*mat2[1][2]+mat1[2][2]*mat2[2][2]);

}


short Op01m;
short Op01Zr;
short Op01Xr;
short Op01Yr;
short Op11m;
short Op11Zr;
short Op11Xr;
short Op11Yr;
short Op21m;
short Op21Zr;
short Op21Xr;
short Op21Yr;
double sc,sc2,sc3;

DSPOp01()
{
   double zr,yr,xr;

   zr = ((double)Op01Zr)*6.2832/65536;
   xr = ((double)Op01Yr)*6.2832/65536;
   yr = ((double)Op01Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc = ((double)Op01m)/32768.0;

   matrixA[0][0]=matrixB[0][0]; matrixA[0][1]=matrixB[0][1]; matrixA[0][2]=matrixB[0][2]; 
   matrixA[1][0]=matrixB[1][0]; matrixA[1][1]=matrixB[1][1]; matrixA[1][2]=matrixB[1][2]; 
   matrixA[2][0]=matrixB[2][0]; matrixA[2][1]=matrixB[2][1]; matrixA[2][2]=matrixB[2][2]; 

   #ifdef DebugDSP1
      Log_Message("OP01 ZR: %d XR: %d YR: %d",Op01Zr,Op01Xr,Op01Yr);
   #endif
}

DSPOp11()
{
   double zr,yr,xr;

   zr = ((double)Op11Zr)*6.2832/65536;
   xr = ((double)Op11Yr)*6.2832/65536;
   yr = ((double)Op11Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc2 = ((double)Op11m)/32768.0;

   matrixA2[0][0]=matrixB[0][0]; matrixA2[0][1]=matrixB[0][1]; matrixA2[0][2]=matrixB[0][2]; 
   matrixA2[1][0]=matrixB[1][0]; matrixA2[1][1]=matrixB[1][1]; matrixA2[1][2]=matrixB[1][2]; 
   matrixA2[2][0]=matrixB[2][0]; matrixA2[2][1]=matrixB[2][1]; matrixA2[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP11 ZR: %d XR: %d YR: %d",Op11Zr,Op11Xr,Op11Yr);
   #endif
}

DSPOp21()
{
   double zr,yr,xr;

   zr = ((double)Op21Zr)*6.2832/65536;
   xr = ((double)Op21Yr)*6.2832/65536;
   yr = ((double)Op21Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc3 = ((double)Op21m)/32768.0;

   matrixA3[0][0]=matrixB[0][0]; matrixA3[0][1]=matrixB[0][1]; matrixA3[0][2]=matrixB[0][2]; 
   matrixA3[1][0]=matrixB[1][0]; matrixA3[1][1]=matrixB[1][1]; matrixA3[1][2]=matrixB[1][2]; 
   matrixA3[2][0]=matrixB[2][0]; matrixA3[2][1]=matrixB[2][1]; matrixA3[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP21 ZR: %d XR: %d YR: %d",Op21Zr,Op21Xr,Op21Yr);
   #endif
}

short Op0DX;
short Op0DY;
short Op0DZ;
short Op0DF;
short Op0DL;
short Op0DU;
short Op1DX;
short Op1DY;
short Op1DZ;
short Op1DF;
short Op1DL;
short Op1DU;
short Op2DX;
short Op2DY;
short Op2DZ;
short Op2DF;
short Op2DL;
short Op2DU;

#define swap(a,b) temp=a;a=b;b=temp;

void DSPOp0D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;

   a = matrixA[0][0]; b=matrixA[0][1]; c=matrixA[0][2];
   d = matrixA[1][0]; e=matrixA[1][1]; f=matrixA[1][2];
   g = matrixA[2][0]; h=matrixA[2][1]; i=matrixA[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op0DF=Op0DX;
     Op0DL=Op0DY;
     Op0DU=Op0DZ;
     #ifdef DebugDSP1
        Log_Message("OP0D Error!  Det == 0");
     #endif
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op0DX; y=Op0DY; z=Op0DZ;
   Op0DF=(short)((x*a2+y*d2+z*g2)/2*sc);
   Op0DL=(short)((x*b2+y*e2+z*h2)/2*sc);
   Op0DU=(short)((x*c2+y*f2+z*i2)/2*sc);

   #ifdef DebugDSP1
      Log_Message("OP0D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op0DX,Op0DY,Op0DZ,Op0DF,Op0DL,Op0DU);
   #endif
}

void DSPOp1D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;
   a = matrixA2[0][0]; b=matrixA2[0][1]; c=matrixA2[0][2];
   d = matrixA2[1][0]; e=matrixA2[1][1]; f=matrixA2[1][2];
   g = matrixA2[2][0]; h=matrixA2[2][1]; i=matrixA2[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op1DF=0; Op1DL=0; Op1DU=0;
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op1DX; y=Op1DY; z=Op1DZ;
   Op1DF=(short)((x*a2+y*d2+z*g2)/2*sc2);
   Op1DL=(short)((x*b2+y*e2+z*h2)/2*sc2);
   Op1DU=(short)((x*c2+y*f2+z*i2)/2*sc2);
   #ifdef DebugDSP1
      Log_Message("OP1D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op1DX,Op1DY,Op1DZ,Op1DF,Op1DL,Op1DU);
   #endif
}

void DSPOp2D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;
   a = matrixA3[0][0]; b=matrixA3[0][1]; c=matrixA3[0][2];
   d = matrixA3[1][0]; e=matrixA3[1][1]; f=matrixA3[1][2];
   g = matrixA3[2][0]; h=matrixA3[2][1]; i=matrixA3[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op2DF=0; Op2DL=0; Op2DU=0;
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op2DX; y=Op2DY; z=Op2DZ;
   Op2DF=(short)((x*a2+y*d2+z*g2)/2*sc3);
   Op2DL=(short)((x*b2+y*e2+z*h2)/2*sc3);
   Op2DU=(short)((x*c2+y*f2+z*i2)/2*sc3);
   #ifdef DebugDSP1
      Log_Message("OP2D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op2DX,Op2DY,Op2DZ,Op2DF,Op2DL,Op2DU);
   #endif
}

short Op03F;
short Op03L;
short Op03U;
short Op03X;
short Op03Y;
short Op03Z;
short Op13F;
short Op13L;
short Op13U;
short Op13X;
short Op13Y;
short Op13Z;
short Op23F;
short Op23L;
short Op23U;
short Op23X;
short Op23Y;
short Op23Z;

void DSPOp03()
{
   double F,L,U;

   F=Op03F; L=Op03L; U=Op03U;
   Op03X=(short)((F*matrixA[0][0]+L*matrixA[1][0]+U*matrixA[2][0])/2*sc);
   Op03Y=(short)((F*matrixA[0][1]+L*matrixA[1][1]+U*matrixA[2][1])/2*sc);
   Op03Z=(short)((F*matrixA[0][2]+L*matrixA[1][2]+U*matrixA[2][2])/2*sc);

   #ifdef DebugDSP1
      Log_Message("OP03 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op03F,Op03L,Op03U,Op03X,Op03Y,Op03Z);
   #endif
}

void DSPOp13()
{
   double F,L,U;
   F=Op13F; L=Op13L; U=Op13U;
   Op13X=(short)((F*matrixA2[0][0]+L*matrixA2[1][0]+U*matrixA2[2][0])/2*sc2);
   Op13Y=(short)((F*matrixA2[0][1]+L*matrixA2[1][1]+U*matrixA2[2][1])/2*sc2);
   Op13Z=(short)((F*matrixA2[0][2]+L*matrixA2[1][2]+U*matrixA2[2][2])/2*sc2);
   #ifdef DebugDSP1
      Log_Message("OP13 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op13F,Op13L,Op13U,Op13X,Op13Y,Op13Z);
   #endif
}

DSPOp23()
{
   double F,L,U;
   F=Op23F; L=Op23L; U=Op23U;
   Op23X=(short)((F*matrixA3[0][0]+L*matrixA3[1][0]+U*matrixA3[2][0])/2*sc3);
   Op23Y=(short)((F*matrixA3[0][1]+L*matrixA3[1][1]+U*matrixA3[2][1])/2*sc3);
   Op23Z=(short)((F*matrixA3[0][2]+L*matrixA3[1][2]+U*matrixA3[2][2])/2*sc3);
   #ifdef DebugDSP1
      Log_Message("OP23 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op23F,Op23L,Op23U,Op23X,Op23Y,Op23Z);
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
   Op14Zrr=(short)(Op14Temp*65536.0/6.2832);
   Op14Temp=(Op14Xr*6.2832/65536.0)+((Op14U*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0));
   Op14Xrr=(short)(Op14Temp*65536.0/6.2832);
   Op14Temp=(Op14Yr*6.2832/65536.0)-tan(Op14Xr*6.2832/65536.0)*((Op14U*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0))+(Op14L*6.2832/65536.0);
   Op14Yrr=(short)(Op14Temp*65536.0/6.2832);
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
   RVPos = Op0EV;
   RHPos = Op0EH;
   GetRXYPos();
   Op0EX = RXRes;
   Op0EY = RYRes;

   #ifdef DebugDSP1
      Log_Message("OP0E COORDINATE H:%d V:%d   X:%d Y:%d",Op0EH,Op0EV,Op0EX,Op0EY);
   #endif
}

short Op0BX;
short Op0BY;
short Op0BZ;
short Op0BS;
short Op1BX;
short Op1BY;
short Op1BZ;
short Op1BS;
short Op2BX;
short Op2BY;
short Op2BZ;
short Op2BS;

void DSPOp0B()
{
	Log_Message("OP0B : NOT IMPLEMENTED");
}

void DSPOp1B()
{   
        Op1BS = (Op1BX*matrixA2[0][0]+Op1BY*matrixA2[0][1]+Op1BZ*matrixA2[0][2])*sc2*50;
#ifdef DebugDSP1
      Log_Message("OP1B X: %d Y: %d Z: %d S: %d",Op1BX,Op1BY,Op1BZ,Op1BS);
#endif

}

void DSPOp2B()
{
#ifdef DebugDSP1
      Log_Message("OP2B : NOT IMPLEMENTED");
#endif
}

short Op08X,Op08Y,Op08Z,Op08Ll,Op08Lh;
long Op08Size;

void DSPOp08()
{
   Op08Size=(Op08X*Op08X+Op08Y*Op08Y+Op08Z*Op08Z)*2;
   Op08Ll = Op08Size&0xFFFF;
   Op08Lh = (Op08Size>>16) & 0xFFFF;
   #ifdef DebugDSP1
      Log_Message("OP08 %d,%d,%d",Op08X,Op08Y,Op08Z);
      Log_Message("OP08 ((Op08X^2)+(Op08Y^2)+(Op08X^2))=%x",Op08Size );
   #endif
}

short Op18X,Op18Y,Op18Z,Op18R,Op18D;

void DSPOp18()
{
   Op18D=(Op18X*Op18X+Op18Y*Op18Y+Op18Z*Op18Z-Op18R*Op18R)/65536;
   #ifdef DebugDSP1
      Log_Message("OP18 X: %d Y: %d Z: %d DIFF %d",Op18X,Op18Y,Op18Z,Op18D);
   #endif
}

short Op28X;
short Op28Y;
short Op28Z;
short Op28R;

DSPOp28()
{
   Op28R=(short)sqrt(Op28X*Op28X+Op28Y*Op28Y+Op28Z*Op28Z);
   #ifdef DebugDSP1
      Log_Message("OP28 X:%d Y:%d Z:%d",Op28X,Op28Y,Op28Z);
      Log_Message("OP28 Vector Length %d",Op28R);
   #endif
}

short Op1CAZ;
unsigned short Op1CX,Op1CY,Op1CZ;
short Op1CXBR,Op1CYBR,Op1CZBR,Op1CXAR,Op1CYAR,Op1CZAR;
short Op1CX1;
short Op1CY1;
short Op1CZ1;
short Op1CX2;
short Op1CY2;
short Op1CZ2;

void DSPOp1C()
{
   double ya,xa,za;
   ya = Op1CX/65536.0*3.1415*2;
   xa = Op1CY/65536.0*3.1415*2;
   za = Op1CZ/65536.0*3.1415*2;
   // rotate around Z
   Op1CX1=(Op1CXBR*cos(za)+Op1CYBR*sin(za));
   Op1CY1=(Op1CXBR*-sin(za)+Op1CYBR*cos(za));
   Op1CZ1=Op1CZBR;
   // rotate around Y
   Op1CX2=(Op1CX1*cos(ya)+Op1CZ1*-sin(ya));
   Op1CY2=Op1CY1;
   Op1CZ2=(Op1CX1*sin(ya)+Op1CZ1*cos(ya));
   // rotate around X
   Op1CXAR=Op1CX2;
   Op1CYAR=(Op1CY2*cos(xa)+Op1CZ2*sin(xa));
   Op1CZAR=(Op1CY2*-sin(xa)+Op1CZ2*cos(xa));

   #ifdef DebugDSP1
      Log_Message("OP1C Apply Matrix CX:%d CY:%d CZ",Op1CXAR,Op1CYAR,Op1CZAR);
   #endif
}

