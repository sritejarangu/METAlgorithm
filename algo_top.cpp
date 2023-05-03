#include "algo_top.h"
#include "cordic.h"
#include <algorithm>
#include <utility>

#include "objects.h"
using namespace std;
using namespace algo;

void unpackInputLink(ap_uint<576> &ilink, Tower towers[TOWERS_IN_ETA/2]) {
#pragma HLS PIPELINE II=9
#pragma HLS ARRAY_PARTITION variable=towers complete dim=0
#pragma HLS INLINE

  ap_uint<576> word_576b_;



  word_576b_ = ilink.read();

  towers[0]  = Tower(word_576b_( 31,   0));
  towers[1]  = Tower(word_576b_( 63,  32));
  towers[2]  = Tower(word_576b_( 95,  64));
  towers[3]  = Tower(word_576b_(127,  96));
  towers[4]  = Tower(word_576b_(159, 128));
  towers[5]  = Tower(word_576b_(191, 160));
  towers[6]  = Tower(word_576b_(223, 192));
  towers[7]  = Tower(word_576b_(255, 224));
  towers[8]  = Tower(word_576b_(287, 256));
  towers[9]  = Tower(word_576b_(319, 288));
  towers[10] = Tower(word_576b_(351, 320));
  towers[11] = Tower(word_576b_(383, 352));
  towers[12] = Tower(word_576b_(415, 384));
  towers[13] = Tower(word_576b_(447, 416));
  towers[14] = Tower(word_576b_(479, 448));
  towers[15] = Tower(word_576b_(511, 480));
  towers[16] = Tower(word_576b_(543, 512));

  return;
}

void packOutput(ap_uint<16> a[0], ap_uint<576> &olink){
#pragma HLS PIPELINE II=9
#pragma HLS ARRAY_PARTITION variable=a complete dim=0
#pragma HLS INLINE

  ap_uint<576> word_576b_;


  word_576b_(15, 0) = (ap_uint<16>) a[0];
  word_576b_(31, 16) = (ap_uint<16>) a[1];
  word_576b_(47, 32) = (ap_uint<16>) a[2];
  word_576b_(63, 48) = (ap_uint<16>) a[3];
  word_576b_(79, 64) = (ap_uint<16>) a[4];
  word_576b_(95, 80) = (ap_uint<16>) a[5];
  word_576b_(111, 96) = (ap_uint<16>) a[6];
  word_576b_(127, 112) = (ap_uint<16>) a[7];
  word_576b_(143, 128) = (ap_uint<16>) a[8];
  word_576b_(159, 144) = (ap_uint<16>) a[9];
  word_576b_(175, 160) = (ap_uint<16>) a[10];
  word_576b_(191, 176) = (ap_uint<16>) a[11];
  word_576b_(207, 192) = (ap_uint<16>) a[12];
  word_576b_(223, 208) = (ap_uint<16>) a[13];
  word_576b_(239, 224) = (ap_uint<16>) a[14];
  word_576b_(255, 240) = (ap_uint<16>) a[15];
  word_576b_(271, 256) = (ap_uint<16>) a[16];
  word_576b_(287, 272) = (ap_uint<16>) a[17];
  word_576b_(303, 288) = (ap_uint<16>) a[18];
  word_576b_(319, 304) = (ap_uint<16>) a[19];
  word_576b_(335, 320) = (ap_uint<16>) a[20];
  word_576b_(351, 336) = (ap_uint<16>) a[21];
  word_576b_(367, 352) = (ap_uint<16>) a[22];
  word_576b_(383, 368) = (ap_uint<16>) a[23];
  word_576b_(575, 384) = 0;
  ap_uint<576> r; //r.last = 0; r.user = 0;
  r = word_576b_;
  
  olink.write(r);

  return ;
}

void algo_top(ap_uint<576> link_in[N_INPUT_LINKS], ap_uint<576> link_out[N_OUTPUT_LINKS]) {

#pragma HLS PIPELINE II=9
#pragma HLS ARRAY_PARTITION variable=link_in complete dim=0
#pragma HLS ARRAY_PARTITION variable=link_out complete dim=0

  // Step 1: Unpack links
  // Input is 64 links carrying 32phix34eta towers
  Tower towersinpos[TOWERS_IN_PHI][TOWERS_IN_ETA/2];
  Tower towersinneg[TOWERS_IN_PHI][TOWERS_IN_ETA/2];
#pragma HLS ARRAY_PARTITION variable=towersinpos complete dim=0
#pragma HLS ARRAY_PARTITION variable=towersinneg complete dim=0

     
  for (size_t ilink = 0; ilink < N_INPUT_LINKS/2; ilink++) {
      #pragma LOOP UNROLL
      #pragma HLS latency min=1
    size_t iPosEta = ilink;
    size_t iNegEta = ilink+N_INPUT_LINKS/2;
    unpackInputLink(link_in[iNegEta], &towersinneg[ilink][0]);
    unpackInputLink(link_in[iPosEta], &towersinpos[ilink][0]);
  }

   // Step 2: MET Algo goes here
  ap_uint<16> Exs[24];
#pragma HLS ARRAY_PARTITION variable=Exs complete dim=0
  ap_uint<16> Eys[24];
#pragma HLS ARRAY_PARTITION variable=Eys complete dim=0
  COS_SIN_TYPE sinphi[angle];
#pragma HLS ARRAY_PARTITION variable=sinphi complete dim=0
  COS_SIN_TYPE cosphi[angle];
#pragma HLS ARRAY_PARTITION variable=cosphi complete dim=0

  for (ap_uint<8> c = 0; c < angle; c++) {
     #pragma hls unroll

     THETA_TYPE  radian= (c+2.5)*0.0174533;     /* sin and cos calculation*/
     cordic(radian, sinphi[c],cosphi[c]);
    }

  for (ap_uint<5> b = 4; b < 28; b++) {
   #pragma hls unroll
   #pragma HLS latency min=1

   ap_uint<8> phi;

	  if (b<36)
	  phi=180*b/36;     /*calulating theta of the respective tower*/
	  else
	  phi= -180*(72-b)/36;
	  //cout<< "phi is"<< phi<< endl;

	  ap_fixed<16,12> Ey; ap_fixed<16,12> Ex; ap_uint<16> j;
	 
	  ap_uint<16> p; ap_uint<16> h;

	  p= towersinpos[b][0].tower_et() + towersinpos[b][1].tower_et() + towersinpos[b][2].tower_et() + towersinpos[b][3].tower_et() + towersinpos[b][4].tower_et()
	     + towersinpos[b][5].tower_et() + towersinpos[b][6].tower_et() + towersinpos[b][7].tower_et() + towersinpos[b][8].tower_et() + towersinpos[b][9].tower_et()
	     + towersinpos[b][10].tower_et() + towersinpos[b][11].tower_et() + towersinpos[b][12].tower_et() + towersinpos[b][13].tower_et() + towersinpos[b][14].tower_et()
	     + towersinpos[b][15].tower_et() + towersinpos[b][16].tower_et() ;

	  h= towersinneg[b][0].tower_et() + towersinneg[b][1].tower_et() + towersinneg[b][2].tower_et() + towersinneg[b][3].tower_et() + towersinneg[b][4].tower_et()
	  + towersinneg[b][5].tower_et() + towersinneg[b][6].tower_et() + towersinneg[b][7].tower_et() + towersinneg[b][8].tower_et() + towersinneg[b][9].tower_et()
	  + towersinneg[b][10].tower_et() + towersinneg[b][11].tower_et() + towersinneg[b][12].tower_et() + towersinneg[b][13].tower_et() + towersinneg[b][14].tower_et()
	  + towersinneg[b][15].tower_et() + towersinneg[b][16].tower_et();

	  j= p + h;

  		Ey = sinphi[phi]*j;
  		//cout << Ey << endl;
  		Eys[b-4] = Ey;
  		Ex = cosphi[phi]*j;
  		Exs[b-4] = Ex;
  		//cout << Ex <<endl;
  	}
  // Step 3: Pack the outputs

    packOutput(&Exs[0],link_out[0]);
    packOutput(&Eys[0],link_out[1]);
}




