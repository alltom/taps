/*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sun Nov  7 20:44:28 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2real 64 */

/*
 * This function contains 394 FP additions, 146 FP multiplications,
 * (or, 342 additions, 94 multiplications, 52 fused multiply/add),
 * 86 stack variables, and 128 memory accesses
 */
static const fftw_real K555570233 = FFTW_KONST(+0.555570233019602224742830813948532874374937191);
static const fftw_real K831469612 = FFTW_KONST(+0.831469612302545237078788377617905756738560812);
static const fftw_real K195090322 = FFTW_KONST(+0.195090322016128267848284868477022240927691618);
static const fftw_real K980785280 = FFTW_KONST(+0.980785280403230449126182236134239036973933731);
static const fftw_real K765366864 = FFTW_KONST(+0.765366864730179543456919968060797733522689125);
static const fftw_real K1_847759065 = FFTW_KONST(+1.847759065022573512256366378793576573644833252);
static const fftw_real K995184726 = FFTW_KONST(+0.995184726672196886244836953109479921575474869);
static const fftw_real K098017140 = FFTW_KONST(+0.098017140329560601994195563888641845861136673);
static const fftw_real K471396736 = FFTW_KONST(+0.471396736825997648556387625905254377657460319);
static const fftw_real K881921264 = FFTW_KONST(+0.881921264348355029712756863660388349508442621);
static const fftw_real K773010453 = FFTW_KONST(+0.773010453362736960810906609758469800971041293);
static const fftw_real K634393284 = FFTW_KONST(+0.634393284163645498215171613225493370675687095);
static const fftw_real K290284677 = FFTW_KONST(+0.290284677254462367636192375817395274691476278);
static const fftw_real K956940335 = FFTW_KONST(+0.956940335732208864935797886980269969482849206);
static const fftw_real K1_414213562 = FFTW_KONST(+1.414213562373095048801688724209698078569671875);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);

/*
 * Generator Id's : 
 * $Id: fcr_64.c,v 1.1.1.1 2004/11/11 22:09:09 am-treesynth Exp $
 * $Id: fcr_64.c,v 1.1.1.1 2004/11/11 22:09:09 am-treesynth Exp $
 * $Id: fcr_64.c,v 1.1.1.1 2004/11/11 22:09:09 am-treesynth Exp $
 */

void fftw_hc2real_64(const fftw_real *real_input, const fftw_real *imag_input, fftw_real *output, int real_istride, int imag_istride, int ostride)
{
     fftw_real tmp10;
     fftw_real tmp196;
     fftw_real tmp70;
     fftw_real tmp152;
     fftw_real tmp227;
     fftw_real tmp301;
     fftw_real tmp327;
     fftw_real tmp373;
     fftw_real tmp64;
     fftw_real tmp206;
     fftw_real tmp128;
     fftw_real tmp163;
     fftw_real tmp184;
     fftw_real tmp204;
     fftw_real tmp137;
     fftw_real tmp162;
     fftw_real tmp273;
     fftw_real tmp287;
     fftw_real tmp349;
     fftw_real tmp359;
     fftw_real tmp280;
     fftw_real tmp288;
     fftw_real tmp352;
     fftw_real tmp360;
     fftw_real tmp17;
     fftw_real tmp79;
     fftw_real tmp153;
     fftw_real tmp197;
     fftw_real tmp234;
     fftw_real tmp302;
     fftw_real tmp330;
     fftw_real tmp374;
     fftw_real tmp25;
     fftw_real tmp32;
     fftw_real tmp199;
     fftw_real tmp89;
     fftw_real tmp155;
     fftw_real tmp187;
     fftw_real tmp188;
     fftw_real tmp200;
     fftw_real tmp98;
     fftw_real tmp156;
     fftw_real tmp242;
     fftw_real tmp292;
     fftw_real tmp334;
     fftw_real tmp364;
     fftw_real tmp249;
     fftw_real tmp293;
     fftw_real tmp337;
     fftw_real tmp365;
     fftw_real tmp49;
     fftw_real tmp203;
     fftw_real tmp109;
     fftw_real tmp159;
     fftw_real tmp181;
     fftw_real tmp207;
     fftw_real tmp118;
     fftw_real tmp160;
     fftw_real tmp258;
     fftw_real tmp284;
     fftw_real tmp342;
     fftw_real tmp356;
     fftw_real tmp265;
     fftw_real tmp285;
     fftw_real tmp345;
     fftw_real tmp357;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp5;
	  fftw_real tmp222;
	  fftw_real tmp3;
	  fftw_real tmp220;
	  fftw_real tmp9;
	  fftw_real tmp224;
	  fftw_real tmp69;
	  fftw_real tmp225;
	  fftw_real tmp6;
	  fftw_real tmp66;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp4;
	       fftw_real tmp221;
	       fftw_real tmp1;
	       fftw_real tmp2;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp4 = real_input[16 * real_istride];
	       tmp5 = K2_000000000 * tmp4;
	       tmp221 = imag_input[16 * imag_istride];
	       tmp222 = K2_000000000 * tmp221;
	       tmp1 = real_input[0];
	       tmp2 = real_input[32 * real_istride];
	       tmp3 = tmp1 + tmp2;
	       tmp220 = tmp1 - tmp2;
	       {
		    fftw_real tmp7;
		    fftw_real tmp8;
		    fftw_real tmp67;
		    fftw_real tmp68;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp7 = real_input[8 * real_istride];
		    tmp8 = real_input[24 * real_istride];
		    tmp9 = K2_000000000 * (tmp7 + tmp8);
		    tmp224 = tmp7 - tmp8;
		    tmp67 = imag_input[8 * imag_istride];
		    tmp68 = imag_input[24 * imag_istride];
		    tmp69 = K2_000000000 * (tmp67 - tmp68);
		    tmp225 = tmp68 + tmp67;
	       }
	  }
	  tmp6 = tmp3 + tmp5;
	  tmp10 = tmp6 + tmp9;
	  tmp196 = tmp6 - tmp9;
	  tmp66 = tmp3 - tmp5;
	  tmp70 = tmp66 - tmp69;
	  tmp152 = tmp66 + tmp69;
	  {
	       fftw_real tmp223;
	       fftw_real tmp226;
	       fftw_real tmp325;
	       fftw_real tmp326;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp223 = tmp220 - tmp222;
	       tmp226 = K1_414213562 * (tmp224 - tmp225);
	       tmp227 = tmp223 + tmp226;
	       tmp301 = tmp223 - tmp226;
	       tmp325 = tmp220 + tmp222;
	       tmp326 = K1_414213562 * (tmp224 + tmp225);
	       tmp327 = tmp325 - tmp326;
	       tmp373 = tmp325 + tmp326;
	  }
     }
     {
	  fftw_real tmp52;
	  fftw_real tmp267;
	  fftw_real tmp135;
	  fftw_real tmp271;
	  fftw_real tmp55;
	  fftw_real tmp270;
	  fftw_real tmp132;
	  fftw_real tmp268;
	  fftw_real tmp59;
	  fftw_real tmp274;
	  fftw_real tmp126;
	  fftw_real tmp278;
	  fftw_real tmp62;
	  fftw_real tmp277;
	  fftw_real tmp123;
	  fftw_real tmp275;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp50;
	       fftw_real tmp51;
	       fftw_real tmp133;
	       fftw_real tmp134;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp50 = real_input[3 * real_istride];
	       tmp51 = real_input[29 * real_istride];
	       tmp52 = tmp50 + tmp51;
	       tmp267 = tmp50 - tmp51;
	       tmp133 = imag_input[3 * imag_istride];
	       tmp134 = imag_input[29 * imag_istride];
	       tmp135 = tmp133 - tmp134;
	       tmp271 = tmp133 + tmp134;
	  }
	  {
	       fftw_real tmp53;
	       fftw_real tmp54;
	       fftw_real tmp130;
	       fftw_real tmp131;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp53 = real_input[13 * real_istride];
	       tmp54 = real_input[19 * real_istride];
	       tmp55 = tmp53 + tmp54;
	       tmp270 = tmp54 - tmp53;
	       tmp130 = imag_input[13 * imag_istride];
	       tmp131 = imag_input[19 * imag_istride];
	       tmp132 = tmp130 - tmp131;
	       tmp268 = tmp131 + tmp130;
	  }
	  {
	       fftw_real tmp57;
	       fftw_real tmp58;
	       fftw_real tmp124;
	       fftw_real tmp125;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp57 = real_input[5 * real_istride];
	       tmp58 = real_input[27 * real_istride];
	       tmp59 = tmp57 + tmp58;
	       tmp274 = tmp57 - tmp58;
	       tmp124 = imag_input[5 * imag_istride];
	       tmp125 = imag_input[27 * imag_istride];
	       tmp126 = tmp124 - tmp125;
	       tmp278 = tmp124 + tmp125;
	  }
	  {
	       fftw_real tmp60;
	       fftw_real tmp61;
	       fftw_real tmp121;
	       fftw_real tmp122;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp60 = real_input[11 * real_istride];
	       tmp61 = real_input[21 * real_istride];
	       tmp62 = tmp60 + tmp61;
	       tmp277 = tmp61 - tmp60;
	       tmp121 = imag_input[11 * imag_istride];
	       tmp122 = imag_input[21 * imag_istride];
	       tmp123 = tmp121 - tmp122;
	       tmp275 = tmp122 + tmp121;
	  }
	  {
	       fftw_real tmp56;
	       fftw_real tmp63;
	       fftw_real tmp120;
	       fftw_real tmp127;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp56 = tmp52 + tmp55;
	       tmp63 = tmp59 + tmp62;
	       tmp64 = tmp56 + tmp63;
	       tmp206 = tmp63 - tmp56;
	       tmp120 = tmp52 - tmp55;
	       tmp127 = tmp123 + tmp126;
	       tmp128 = tmp120 - tmp127;
	       tmp163 = tmp120 + tmp127;
	  }
	  {
	       fftw_real tmp182;
	       fftw_real tmp183;
	       fftw_real tmp129;
	       fftw_real tmp136;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp182 = tmp135 - tmp132;
	       tmp183 = tmp126 - tmp123;
	       tmp184 = tmp182 - tmp183;
	       tmp204 = tmp182 + tmp183;
	       tmp129 = tmp62 - tmp59;
	       tmp136 = tmp132 + tmp135;
	       tmp137 = tmp129 + tmp136;
	       tmp162 = tmp136 - tmp129;
	  }
	  {
	       fftw_real tmp269;
	       fftw_real tmp272;
	       fftw_real tmp347;
	       fftw_real tmp348;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp269 = tmp267 - tmp268;
	       tmp272 = tmp270 + tmp271;
	       tmp273 = (K956940335 * tmp269) - (K290284677 * tmp272);
	       tmp287 = (K290284677 * tmp269) + (K956940335 * tmp272);
	       tmp347 = tmp267 + tmp268;
	       tmp348 = tmp271 - tmp270;
	       tmp349 = (K634393284 * tmp347) - (K773010453 * tmp348);
	       tmp359 = (K773010453 * tmp347) + (K634393284 * tmp348);
	  }
	  {
	       fftw_real tmp276;
	       fftw_real tmp279;
	       fftw_real tmp350;
	       fftw_real tmp351;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp276 = tmp274 - tmp275;
	       tmp279 = tmp277 + tmp278;
	       tmp280 = (K881921264 * tmp276) - (K471396736 * tmp279);
	       tmp288 = (K471396736 * tmp276) + (K881921264 * tmp279);
	       tmp350 = tmp274 + tmp275;
	       tmp351 = tmp278 - tmp277;
	       tmp352 = (K098017140 * tmp350) - (K995184726 * tmp351);
	       tmp360 = (K995184726 * tmp350) + (K098017140 * tmp351);
	  }
     }
     {
	  fftw_real tmp13;
	  fftw_real tmp228;
	  fftw_real tmp77;
	  fftw_real tmp232;
	  fftw_real tmp16;
	  fftw_real tmp231;
	  fftw_real tmp74;
	  fftw_real tmp229;
	  fftw_real tmp71;
	  fftw_real tmp78;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp11;
	       fftw_real tmp12;
	       fftw_real tmp75;
	       fftw_real tmp76;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp11 = real_input[4 * real_istride];
	       tmp12 = real_input[28 * real_istride];
	       tmp13 = tmp11 + tmp12;
	       tmp228 = tmp11 - tmp12;
	       tmp75 = imag_input[4 * imag_istride];
	       tmp76 = imag_input[28 * imag_istride];
	       tmp77 = tmp75 - tmp76;
	       tmp232 = tmp75 + tmp76;
	  }
	  {
	       fftw_real tmp14;
	       fftw_real tmp15;
	       fftw_real tmp72;
	       fftw_real tmp73;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp14 = real_input[12 * real_istride];
	       tmp15 = real_input[20 * real_istride];
	       tmp16 = tmp14 + tmp15;
	       tmp231 = tmp15 - tmp14;
	       tmp72 = imag_input[12 * imag_istride];
	       tmp73 = imag_input[20 * imag_istride];
	       tmp74 = tmp72 - tmp73;
	       tmp229 = tmp73 + tmp72;
	  }
	  tmp17 = K2_000000000 * (tmp13 + tmp16);
	  tmp71 = tmp13 - tmp16;
	  tmp78 = tmp74 + tmp77;
	  tmp79 = K1_414213562 * (tmp71 - tmp78);
	  tmp153 = K1_414213562 * (tmp71 + tmp78);
	  tmp197 = K2_000000000 * (tmp77 - tmp74);
	  {
	       fftw_real tmp230;
	       fftw_real tmp233;
	       fftw_real tmp328;
	       fftw_real tmp329;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp230 = tmp228 - tmp229;
	       tmp233 = tmp231 + tmp232;
	       tmp234 = (K1_847759065 * tmp230) - (K765366864 * tmp233);
	       tmp302 = (K765366864 * tmp230) + (K1_847759065 * tmp233);
	       tmp328 = tmp228 + tmp229;
	       tmp329 = tmp232 - tmp231;
	       tmp330 = (K765366864 * tmp328) - (K1_847759065 * tmp329);
	       tmp374 = (K1_847759065 * tmp328) + (K765366864 * tmp329);
	  }
     }
     {
	  fftw_real tmp21;
	  fftw_real tmp236;
	  fftw_real tmp96;
	  fftw_real tmp240;
	  fftw_real tmp24;
	  fftw_real tmp239;
	  fftw_real tmp93;
	  fftw_real tmp237;
	  fftw_real tmp28;
	  fftw_real tmp243;
	  fftw_real tmp87;
	  fftw_real tmp247;
	  fftw_real tmp31;
	  fftw_real tmp246;
	  fftw_real tmp84;
	  fftw_real tmp244;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp19;
	       fftw_real tmp20;
	       fftw_real tmp94;
	       fftw_real tmp95;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp19 = real_input[2 * real_istride];
	       tmp20 = real_input[30 * real_istride];
	       tmp21 = tmp19 + tmp20;
	       tmp236 = tmp19 - tmp20;
	       tmp94 = imag_input[2 * imag_istride];
	       tmp95 = imag_input[30 * imag_istride];
	       tmp96 = tmp94 - tmp95;
	       tmp240 = tmp94 + tmp95;
	  }
	  {
	       fftw_real tmp22;
	       fftw_real tmp23;
	       fftw_real tmp91;
	       fftw_real tmp92;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp22 = real_input[14 * real_istride];
	       tmp23 = real_input[18 * real_istride];
	       tmp24 = tmp22 + tmp23;
	       tmp239 = tmp23 - tmp22;
	       tmp91 = imag_input[14 * imag_istride];
	       tmp92 = imag_input[18 * imag_istride];
	       tmp93 = tmp91 - tmp92;
	       tmp237 = tmp92 + tmp91;
	  }
	  {
	       fftw_real tmp26;
	       fftw_real tmp27;
	       fftw_real tmp85;
	       fftw_real tmp86;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp26 = real_input[6 * real_istride];
	       tmp27 = real_input[26 * real_istride];
	       tmp28 = tmp26 + tmp27;
	       tmp243 = tmp26 - tmp27;
	       tmp85 = imag_input[6 * imag_istride];
	       tmp86 = imag_input[26 * imag_istride];
	       tmp87 = tmp85 - tmp86;
	       tmp247 = tmp85 + tmp86;
	  }
	  {
	       fftw_real tmp29;
	       fftw_real tmp30;
	       fftw_real tmp82;
	       fftw_real tmp83;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp29 = real_input[10 * real_istride];
	       tmp30 = real_input[22 * real_istride];
	       tmp31 = tmp29 + tmp30;
	       tmp246 = tmp30 - tmp29;
	       tmp82 = imag_input[10 * imag_istride];
	       tmp83 = imag_input[22 * imag_istride];
	       tmp84 = tmp82 - tmp83;
	       tmp244 = tmp83 + tmp82;
	  }
	  {
	       fftw_real tmp81;
	       fftw_real tmp88;
	       fftw_real tmp90;
	       fftw_real tmp97;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp25 = tmp21 + tmp24;
	       tmp32 = tmp28 + tmp31;
	       tmp199 = tmp25 - tmp32;
	       tmp81 = tmp21 - tmp24;
	       tmp88 = tmp84 + tmp87;
	       tmp89 = tmp81 - tmp88;
	       tmp155 = tmp81 + tmp88;
	       tmp187 = tmp96 - tmp93;
	       tmp188 = tmp87 - tmp84;
	       tmp200 = tmp187 + tmp188;
	       tmp90 = tmp31 - tmp28;
	       tmp97 = tmp93 + tmp96;
	       tmp98 = tmp90 + tmp97;
	       tmp156 = tmp97 - tmp90;
	  }
	  {
	       fftw_real tmp238;
	       fftw_real tmp241;
	       fftw_real tmp332;
	       fftw_real tmp333;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp238 = tmp236 - tmp237;
	       tmp241 = tmp239 + tmp240;
	       tmp242 = (K980785280 * tmp238) - (K195090322 * tmp241);
	       tmp292 = (K195090322 * tmp238) + (K980785280 * tmp241);
	       tmp332 = tmp236 + tmp237;
	       tmp333 = tmp240 - tmp239;
	       tmp334 = (K831469612 * tmp332) - (K555570233 * tmp333);
	       tmp364 = (K555570233 * tmp332) + (K831469612 * tmp333);
	  }
	  {
	       fftw_real tmp245;
	       fftw_real tmp248;
	       fftw_real tmp335;
	       fftw_real tmp336;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp245 = tmp243 - tmp244;
	       tmp248 = tmp246 + tmp247;
	       tmp249 = (K831469612 * tmp245) - (K555570233 * tmp248);
	       tmp293 = (K555570233 * tmp245) + (K831469612 * tmp248);
	       tmp335 = tmp243 + tmp244;
	       tmp336 = tmp247 - tmp246;
	       tmp337 = (K195090322 * tmp335) + (K980785280 * tmp336);
	       tmp365 = (K980785280 * tmp335) - (K195090322 * tmp336);
	  }
     }
     {
	  fftw_real tmp37;
	  fftw_real tmp252;
	  fftw_real tmp116;
	  fftw_real tmp256;
	  fftw_real tmp40;
	  fftw_real tmp255;
	  fftw_real tmp113;
	  fftw_real tmp253;
	  fftw_real tmp44;
	  fftw_real tmp259;
	  fftw_real tmp107;
	  fftw_real tmp263;
	  fftw_real tmp47;
	  fftw_real tmp262;
	  fftw_real tmp104;
	  fftw_real tmp260;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp35;
	       fftw_real tmp36;
	       fftw_real tmp114;
	       fftw_real tmp115;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp35 = real_input[real_istride];
	       tmp36 = real_input[31 * real_istride];
	       tmp37 = tmp35 + tmp36;
	       tmp252 = tmp35 - tmp36;
	       tmp114 = imag_input[imag_istride];
	       tmp115 = imag_input[31 * imag_istride];
	       tmp116 = tmp114 - tmp115;
	       tmp256 = tmp114 + tmp115;
	  }
	  {
	       fftw_real tmp38;
	       fftw_real tmp39;
	       fftw_real tmp111;
	       fftw_real tmp112;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp38 = real_input[15 * real_istride];
	       tmp39 = real_input[17 * real_istride];
	       tmp40 = tmp38 + tmp39;
	       tmp255 = tmp39 - tmp38;
	       tmp111 = imag_input[15 * imag_istride];
	       tmp112 = imag_input[17 * imag_istride];
	       tmp113 = tmp111 - tmp112;
	       tmp253 = tmp112 + tmp111;
	  }
	  {
	       fftw_real tmp42;
	       fftw_real tmp43;
	       fftw_real tmp105;
	       fftw_real tmp106;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp42 = real_input[7 * real_istride];
	       tmp43 = real_input[25 * real_istride];
	       tmp44 = tmp42 + tmp43;
	       tmp259 = tmp42 - tmp43;
	       tmp105 = imag_input[7 * imag_istride];
	       tmp106 = imag_input[25 * imag_istride];
	       tmp107 = tmp105 - tmp106;
	       tmp263 = tmp105 + tmp106;
	  }
	  {
	       fftw_real tmp45;
	       fftw_real tmp46;
	       fftw_real tmp102;
	       fftw_real tmp103;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp45 = real_input[9 * real_istride];
	       tmp46 = real_input[23 * real_istride];
	       tmp47 = tmp45 + tmp46;
	       tmp262 = tmp46 - tmp45;
	       tmp102 = imag_input[9 * imag_istride];
	       tmp103 = imag_input[23 * imag_istride];
	       tmp104 = tmp102 - tmp103;
	       tmp260 = tmp103 + tmp102;
	  }
	  {
	       fftw_real tmp41;
	       fftw_real tmp48;
	       fftw_real tmp101;
	       fftw_real tmp108;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp41 = tmp37 + tmp40;
	       tmp48 = tmp44 + tmp47;
	       tmp49 = tmp41 + tmp48;
	       tmp203 = tmp41 - tmp48;
	       tmp101 = tmp37 - tmp40;
	       tmp108 = tmp104 + tmp107;
	       tmp109 = tmp101 - tmp108;
	       tmp159 = tmp101 + tmp108;
	  }
	  {
	       fftw_real tmp179;
	       fftw_real tmp180;
	       fftw_real tmp110;
	       fftw_real tmp117;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp179 = tmp116 - tmp113;
	       tmp180 = tmp107 - tmp104;
	       tmp181 = tmp179 - tmp180;
	       tmp207 = tmp179 + tmp180;
	       tmp110 = tmp47 - tmp44;
	       tmp117 = tmp113 + tmp116;
	       tmp118 = tmp110 + tmp117;
	       tmp160 = tmp117 - tmp110;
	  }
	  {
	       fftw_real tmp254;
	       fftw_real tmp257;
	       fftw_real tmp340;
	       fftw_real tmp341;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp254 = tmp252 - tmp253;
	       tmp257 = tmp255 + tmp256;
	       tmp258 = (K995184726 * tmp254) - (K098017140 * tmp257);
	       tmp284 = (K098017140 * tmp254) + (K995184726 * tmp257);
	       tmp340 = tmp252 + tmp253;
	       tmp341 = tmp256 - tmp255;
	       tmp342 = (K956940335 * tmp340) - (K290284677 * tmp341);
	       tmp356 = (K290284677 * tmp340) + (K956940335 * tmp341);
	  }
	  {
	       fftw_real tmp261;
	       fftw_real tmp264;
	       fftw_real tmp343;
	       fftw_real tmp344;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp261 = tmp259 - tmp260;
	       tmp264 = tmp262 + tmp263;
	       tmp265 = (K773010453 * tmp261) - (K634393284 * tmp264);
	       tmp285 = (K634393284 * tmp261) + (K773010453 * tmp264);
	       tmp343 = tmp259 + tmp260;
	       tmp344 = tmp263 - tmp262;
	       tmp345 = (K471396736 * tmp343) + (K881921264 * tmp344);
	       tmp357 = (K881921264 * tmp343) - (K471396736 * tmp344);
	  }
     }
     {
	  fftw_real tmp65;
	  fftw_real tmp185;
	  fftw_real tmp34;
	  fftw_real tmp178;
	  fftw_real tmp18;
	  fftw_real tmp33;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp65 = K2_000000000 * (tmp49 + tmp64);
	  tmp185 = K2_000000000 * (tmp181 - tmp184);
	  tmp18 = tmp10 + tmp17;
	  tmp33 = K2_000000000 * (tmp25 + tmp32);
	  tmp34 = tmp18 + tmp33;
	  tmp178 = tmp18 - tmp33;
	  output[32 * ostride] = tmp34 - tmp65;
	  output[0] = tmp34 + tmp65;
	  output[16 * ostride] = tmp178 - tmp185;
	  output[48 * ostride] = tmp178 + tmp185;
     }
     {
	  fftw_real tmp190;
	  fftw_real tmp194;
	  fftw_real tmp193;
	  fftw_real tmp195;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp186;
	       fftw_real tmp189;
	       fftw_real tmp191;
	       fftw_real tmp192;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp186 = tmp10 - tmp17;
	       tmp189 = K2_000000000 * (tmp187 - tmp188);
	       tmp190 = tmp186 - tmp189;
	       tmp194 = tmp186 + tmp189;
	       tmp191 = tmp49 - tmp64;
	       tmp192 = tmp181 + tmp184;
	       tmp193 = K1_414213562 * (tmp191 - tmp192);
	       tmp195 = K1_414213562 * (tmp191 + tmp192);
	  }
	  output[40 * ostride] = tmp190 - tmp193;
	  output[8 * ostride] = tmp190 + tmp193;
	  output[24 * ostride] = tmp194 - tmp195;
	  output[56 * ostride] = tmp194 + tmp195;
     }
     {
	  fftw_real tmp100;
	  fftw_real tmp140;
	  fftw_real tmp146;
	  fftw_real tmp150;
	  fftw_real tmp139;
	  fftw_real tmp147;
	  fftw_real tmp143;
	  fftw_real tmp148;
	  fftw_real tmp151;
	  fftw_real tmp149;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp80;
	       fftw_real tmp99;
	       fftw_real tmp144;
	       fftw_real tmp145;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp80 = tmp70 + tmp79;
	       tmp99 = (K1_847759065 * tmp89) - (K765366864 * tmp98);
	       tmp100 = tmp80 + tmp99;
	       tmp140 = tmp80 - tmp99;
	       tmp144 = tmp70 - tmp79;
	       tmp145 = (K1_847759065 * tmp98) + (K765366864 * tmp89);
	       tmp146 = tmp144 - tmp145;
	       tmp150 = tmp144 + tmp145;
	  }
	  {
	       fftw_real tmp119;
	       fftw_real tmp138;
	       fftw_real tmp141;
	       fftw_real tmp142;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp119 = (K980785280 * tmp109) - (K195090322 * tmp118);
	       tmp138 = (K831469612 * tmp128) - (K555570233 * tmp137);
	       tmp139 = K2_000000000 * (tmp119 + tmp138);
	       tmp147 = tmp119 - tmp138;
	       tmp141 = (K980785280 * tmp118) + (K195090322 * tmp109);
	       tmp142 = (K831469612 * tmp137) + (K555570233 * tmp128);
	       tmp143 = K2_000000000 * (tmp141 - tmp142);
	       tmp148 = tmp141 + tmp142;
	  }
	  output[34 * ostride] = tmp100 - tmp139;
	  output[2 * ostride] = tmp100 + tmp139;
	  output[18 * ostride] = tmp140 - tmp143;
	  output[50 * ostride] = tmp140 + tmp143;
	  tmp151 = K1_414213562 * (tmp147 + tmp148);
	  output[26 * ostride] = tmp150 - tmp151;
	  output[58 * ostride] = tmp150 + tmp151;
	  tmp149 = K1_414213562 * (tmp147 - tmp148);
	  output[42 * ostride] = tmp146 - tmp149;
	  output[10 * ostride] = tmp146 + tmp149;
     }
     {
	  fftw_real tmp339;
	  fftw_real tmp355;
	  fftw_real tmp367;
	  fftw_real tmp371;
	  fftw_real tmp354;
	  fftw_real tmp368;
	  fftw_real tmp362;
	  fftw_real tmp369;
	  fftw_real tmp372;
	  fftw_real tmp370;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp331;
	       fftw_real tmp338;
	       fftw_real tmp363;
	       fftw_real tmp366;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp331 = tmp327 + tmp330;
	       tmp338 = K2_000000000 * (tmp334 - tmp337);
	       tmp339 = tmp331 + tmp338;
	       tmp355 = tmp331 - tmp338;
	       tmp363 = tmp327 - tmp330;
	       tmp366 = K2_000000000 * (tmp364 - tmp365);
	       tmp367 = tmp363 - tmp366;
	       tmp371 = tmp363 + tmp366;
	  }
	  {
	       fftw_real tmp346;
	       fftw_real tmp353;
	       fftw_real tmp358;
	       fftw_real tmp361;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp346 = tmp342 - tmp345;
	       tmp353 = tmp349 + tmp352;
	       tmp354 = K2_000000000 * (tmp346 + tmp353);
	       tmp368 = tmp346 - tmp353;
	       tmp358 = tmp356 - tmp357;
	       tmp361 = tmp359 - tmp360;
	       tmp362 = K2_000000000 * (tmp358 - tmp361);
	       tmp369 = tmp358 + tmp361;
	  }
	  output[35 * ostride] = tmp339 - tmp354;
	  output[3 * ostride] = tmp339 + tmp354;
	  output[19 * ostride] = tmp355 - tmp362;
	  output[51 * ostride] = tmp355 + tmp362;
	  tmp372 = K1_414213562 * (tmp368 + tmp369);
	  output[27 * ostride] = tmp371 - tmp372;
	  output[59 * ostride] = tmp371 + tmp372;
	  tmp370 = K1_414213562 * (tmp368 - tmp369);
	  output[43 * ostride] = tmp367 - tmp370;
	  output[11 * ostride] = tmp367 + tmp370;
     }
     {
	  fftw_real tmp375;
	  fftw_real tmp389;
	  fftw_real tmp378;
	  fftw_real tmp390;
	  fftw_real tmp382;
	  fftw_real tmp392;
	  fftw_real tmp385;
	  fftw_real tmp393;
	  fftw_real tmp376;
	  fftw_real tmp377;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp375 = tmp373 - tmp374;
	  tmp389 = tmp373 + tmp374;
	  tmp376 = tmp334 + tmp337;
	  tmp377 = tmp364 + tmp365;
	  tmp378 = K1_414213562 * (tmp376 - tmp377);
	  tmp390 = K1_414213562 * (tmp377 + tmp376);
	  {
	       fftw_real tmp380;
	       fftw_real tmp381;
	       fftw_real tmp383;
	       fftw_real tmp384;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp380 = tmp342 + tmp345;
	       tmp381 = tmp359 + tmp360;
	       tmp382 = tmp380 - tmp381;
	       tmp392 = tmp380 + tmp381;
	       tmp383 = tmp356 + tmp357;
	       tmp384 = tmp352 - tmp349;
	       tmp385 = tmp383 + tmp384;
	       tmp393 = tmp383 - tmp384;
	  }
	  {
	       fftw_real tmp379;
	       fftw_real tmp386;
	       fftw_real tmp387;
	       fftw_real tmp388;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp379 = tmp375 + tmp378;
	       tmp386 = (K1_847759065 * tmp382) - (K765366864 * tmp385);
	       output[39 * ostride] = tmp379 - tmp386;
	       output[7 * ostride] = tmp379 + tmp386;
	       tmp387 = tmp375 - tmp378;
	       tmp388 = (K1_847759065 * tmp385) + (K765366864 * tmp382);
	       output[23 * ostride] = tmp387 - tmp388;
	       output[55 * ostride] = tmp387 + tmp388;
	  }
	  {
	       fftw_real tmp391;
	       fftw_real tmp394;
	       fftw_real tmp395;
	       fftw_real tmp396;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp391 = tmp389 - tmp390;
	       tmp394 = (K765366864 * tmp392) - (K1_847759065 * tmp393);
	       output[47 * ostride] = tmp391 - tmp394;
	       output[15 * ostride] = tmp391 + tmp394;
	       tmp395 = tmp389 + tmp390;
	       tmp396 = (K765366864 * tmp393) + (K1_847759065 * tmp392);
	       output[31 * ostride] = tmp395 - tmp396;
	       output[63 * ostride] = tmp395 + tmp396;
	  }
     }
     {
	  fftw_real tmp214;
	  fftw_real tmp218;
	  fftw_real tmp217;
	  fftw_real tmp219;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp212;
	       fftw_real tmp213;
	       fftw_real tmp215;
	       fftw_real tmp216;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp212 = tmp196 + tmp197;
	       tmp213 = K1_414213562 * (tmp199 + tmp200);
	       tmp214 = tmp212 - tmp213;
	       tmp218 = tmp212 + tmp213;
	       tmp215 = tmp203 + tmp204;
	       tmp216 = tmp207 - tmp206;
	       tmp217 = (K765366864 * tmp215) - (K1_847759065 * tmp216);
	       tmp219 = (K765366864 * tmp216) + (K1_847759065 * tmp215);
	  }
	  output[44 * ostride] = tmp214 - tmp217;
	  output[12 * ostride] = tmp214 + tmp217;
	  output[28 * ostride] = tmp218 - tmp219;
	  output[60 * ostride] = tmp218 + tmp219;
     }
     {
	  fftw_real tmp202;
	  fftw_real tmp210;
	  fftw_real tmp209;
	  fftw_real tmp211;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp198;
	       fftw_real tmp201;
	       fftw_real tmp205;
	       fftw_real tmp208;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp198 = tmp196 - tmp197;
	       tmp201 = K1_414213562 * (tmp199 - tmp200);
	       tmp202 = tmp198 + tmp201;
	       tmp210 = tmp198 - tmp201;
	       tmp205 = tmp203 - tmp204;
	       tmp208 = tmp206 + tmp207;
	       tmp209 = (K1_847759065 * tmp205) - (K765366864 * tmp208);
	       tmp211 = (K1_847759065 * tmp208) + (K765366864 * tmp205);
	  }
	  output[36 * ostride] = tmp202 - tmp209;
	  output[4 * ostride] = tmp202 + tmp209;
	  output[20 * ostride] = tmp210 - tmp211;
	  output[52 * ostride] = tmp210 + tmp211;
     }
     {
	  fftw_real tmp158;
	  fftw_real tmp166;
	  fftw_real tmp172;
	  fftw_real tmp176;
	  fftw_real tmp165;
	  fftw_real tmp173;
	  fftw_real tmp169;
	  fftw_real tmp174;
	  fftw_real tmp177;
	  fftw_real tmp175;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp154;
	       fftw_real tmp157;
	       fftw_real tmp170;
	       fftw_real tmp171;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp154 = tmp152 - tmp153;
	       tmp157 = (K765366864 * tmp155) - (K1_847759065 * tmp156);
	       tmp158 = tmp154 + tmp157;
	       tmp166 = tmp154 - tmp157;
	       tmp170 = tmp152 + tmp153;
	       tmp171 = (K765366864 * tmp156) + (K1_847759065 * tmp155);
	       tmp172 = tmp170 - tmp171;
	       tmp176 = tmp170 + tmp171;
	  }
	  {
	       fftw_real tmp161;
	       fftw_real tmp164;
	       fftw_real tmp167;
	       fftw_real tmp168;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp161 = (K831469612 * tmp159) - (K555570233 * tmp160);
	       tmp164 = (K980785280 * tmp162) + (K195090322 * tmp163);
	       tmp165 = K2_000000000 * (tmp161 - tmp164);
	       tmp173 = tmp161 + tmp164;
	       tmp167 = (K831469612 * tmp160) + (K555570233 * tmp159);
	       tmp168 = (K980785280 * tmp163) - (K195090322 * tmp162);
	       tmp169 = K2_000000000 * (tmp167 - tmp168);
	       tmp174 = tmp167 + tmp168;
	  }
	  output[38 * ostride] = tmp158 - tmp165;
	  output[6 * ostride] = tmp158 + tmp165;
	  output[22 * ostride] = tmp166 - tmp169;
	  output[54 * ostride] = tmp166 + tmp169;
	  tmp177 = K1_414213562 * (tmp173 + tmp174);
	  output[30 * ostride] = tmp176 - tmp177;
	  output[62 * ostride] = tmp176 + tmp177;
	  tmp175 = K1_414213562 * (tmp173 - tmp174);
	  output[46 * ostride] = tmp172 - tmp175;
	  output[14 * ostride] = tmp172 + tmp175;
     }
     {
	  fftw_real tmp251;
	  fftw_real tmp283;
	  fftw_real tmp295;
	  fftw_real tmp299;
	  fftw_real tmp282;
	  fftw_real tmp296;
	  fftw_real tmp290;
	  fftw_real tmp297;
	  fftw_real tmp300;
	  fftw_real tmp298;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp235;
	       fftw_real tmp250;
	       fftw_real tmp291;
	       fftw_real tmp294;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp235 = tmp227 + tmp234;
	       tmp250 = K2_000000000 * (tmp242 + tmp249);
	       tmp251 = tmp235 + tmp250;
	       tmp283 = tmp235 - tmp250;
	       tmp291 = tmp227 - tmp234;
	       tmp294 = K2_000000000 * (tmp292 - tmp293);
	       tmp295 = tmp291 - tmp294;
	       tmp299 = tmp291 + tmp294;
	  }
	  {
	       fftw_real tmp266;
	       fftw_real tmp281;
	       fftw_real tmp286;
	       fftw_real tmp289;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp266 = tmp258 + tmp265;
	       tmp281 = tmp273 + tmp280;
	       tmp282 = K2_000000000 * (tmp266 + tmp281);
	       tmp296 = tmp266 - tmp281;
	       tmp286 = tmp284 - tmp285;
	       tmp289 = tmp287 - tmp288;
	       tmp290 = K2_000000000 * (tmp286 - tmp289);
	       tmp297 = tmp286 + tmp289;
	  }
	  output[33 * ostride] = tmp251 - tmp282;
	  output[ostride] = tmp251 + tmp282;
	  output[17 * ostride] = tmp283 - tmp290;
	  output[49 * ostride] = tmp283 + tmp290;
	  tmp300 = K1_414213562 * (tmp296 + tmp297);
	  output[25 * ostride] = tmp299 - tmp300;
	  output[57 * ostride] = tmp299 + tmp300;
	  tmp298 = K1_414213562 * (tmp296 - tmp297);
	  output[41 * ostride] = tmp295 - tmp298;
	  output[9 * ostride] = tmp295 + tmp298;
     }
     {
	  fftw_real tmp303;
	  fftw_real tmp317;
	  fftw_real tmp306;
	  fftw_real tmp318;
	  fftw_real tmp310;
	  fftw_real tmp320;
	  fftw_real tmp313;
	  fftw_real tmp321;
	  fftw_real tmp304;
	  fftw_real tmp305;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp303 = tmp301 - tmp302;
	  tmp317 = tmp301 + tmp302;
	  tmp304 = tmp242 - tmp249;
	  tmp305 = tmp292 + tmp293;
	  tmp306 = K1_414213562 * (tmp304 - tmp305);
	  tmp318 = K1_414213562 * (tmp304 + tmp305);
	  {
	       fftw_real tmp308;
	       fftw_real tmp309;
	       fftw_real tmp311;
	       fftw_real tmp312;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp308 = tmp258 - tmp265;
	       tmp309 = tmp287 + tmp288;
	       tmp310 = tmp308 - tmp309;
	       tmp320 = tmp308 + tmp309;
	       tmp311 = tmp284 + tmp285;
	       tmp312 = tmp280 - tmp273;
	       tmp313 = tmp311 + tmp312;
	       tmp321 = tmp311 - tmp312;
	  }
	  {
	       fftw_real tmp307;
	       fftw_real tmp314;
	       fftw_real tmp315;
	       fftw_real tmp316;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp307 = tmp303 + tmp306;
	       tmp314 = (K1_847759065 * tmp310) - (K765366864 * tmp313);
	       output[37 * ostride] = tmp307 - tmp314;
	       output[5 * ostride] = tmp307 + tmp314;
	       tmp315 = tmp303 - tmp306;
	       tmp316 = (K1_847759065 * tmp313) + (K765366864 * tmp310);
	       output[21 * ostride] = tmp315 - tmp316;
	       output[53 * ostride] = tmp315 + tmp316;
	  }
	  {
	       fftw_real tmp319;
	       fftw_real tmp322;
	       fftw_real tmp323;
	       fftw_real tmp324;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp319 = tmp317 - tmp318;
	       tmp322 = (K765366864 * tmp320) - (K1_847759065 * tmp321);
	       output[45 * ostride] = tmp319 - tmp322;
	       output[13 * ostride] = tmp319 + tmp322;
	       tmp323 = tmp317 + tmp318;
	       tmp324 = (K765366864 * tmp321) + (K1_847759065 * tmp320);
	       output[29 * ostride] = tmp323 - tmp324;
	       output[61 * ostride] = tmp323 + tmp324;
	  }
     }
}

fftw_codelet_desc fftw_hc2real_64_desc =
{
     "fftw_hc2real_64",
     (void (*)()) fftw_hc2real_64,
     64,
     FFTW_BACKWARD,
     FFTW_HC2REAL,
     1423,
     0,
     (const int *) 0,
};
