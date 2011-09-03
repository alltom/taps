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
/* Generated on Sun Nov  7 20:45:01 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -twiddleinv 16 */

/*
 * This function contains 174 FP additions, 84 FP multiplications,
 * (or, 136 additions, 46 multiplications, 38 fused multiply/add),
 * 50 stack variables, and 64 memory accesses
 */
static const fftw_real K382683432 = FFTW_KONST(+0.382683432365089771728459984030398866761344562);
static const fftw_real K923879532 = FFTW_KONST(+0.923879532511286756128183189396788286822416626);
static const fftw_real K707106781 = FFTW_KONST(+0.707106781186547524400844362104849039284835938);

/*
 * Generator Id's : 
 * $Id: ftwi_16.c,v 1.1.1.1 2004/11/11 22:09:10 am-treesynth Exp $
 * $Id: ftwi_16.c,v 1.1.1.1 2004/11/11 22:09:10 am-treesynth Exp $
 * $Id: ftwi_16.c,v 1.1.1.1 2004/11/11 22:09:10 am-treesynth Exp $
 */

void fftwi_twiddle_16(fftw_complex *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 15) {
	  fftw_real tmp7;
	  fftw_real tmp91;
	  fftw_real tmp180;
	  fftw_real tmp194;
	  fftw_real tmp18;
	  fftw_real tmp193;
	  fftw_real tmp94;
	  fftw_real tmp177;
	  fftw_real tmp77;
	  fftw_real tmp88;
	  fftw_real tmp161;
	  fftw_real tmp117;
	  fftw_real tmp141;
	  fftw_real tmp162;
	  fftw_real tmp163;
	  fftw_real tmp164;
	  fftw_real tmp112;
	  fftw_real tmp140;
	  fftw_real tmp30;
	  fftw_real tmp153;
	  fftw_real tmp100;
	  fftw_real tmp137;
	  fftw_real tmp41;
	  fftw_real tmp152;
	  fftw_real tmp105;
	  fftw_real tmp136;
	  fftw_real tmp54;
	  fftw_real tmp65;
	  fftw_real tmp156;
	  fftw_real tmp128;
	  fftw_real tmp144;
	  fftw_real tmp157;
	  fftw_real tmp158;
	  fftw_real tmp159;
	  fftw_real tmp123;
	  fftw_real tmp143;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp179;
	       fftw_real tmp6;
	       fftw_real tmp178;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp179 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[8 * iostride]);
		    tmp5 = c_im(inout[8 * iostride]);
		    tmp2 = c_re(W[7]);
		    tmp4 = c_im(W[7]);
		    tmp6 = (tmp2 * tmp3) + (tmp4 * tmp5);
		    tmp178 = (tmp2 * tmp5) - (tmp4 * tmp3);
	       }
	       tmp7 = tmp1 + tmp6;
	       tmp91 = tmp1 - tmp6;
	       tmp180 = tmp178 + tmp179;
	       tmp194 = tmp179 - tmp178;
	  }
	  {
	       fftw_real tmp12;
	       fftw_real tmp92;
	       fftw_real tmp17;
	       fftw_real tmp93;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp9;
		    fftw_real tmp11;
		    fftw_real tmp8;
		    fftw_real tmp10;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp9 = c_re(inout[4 * iostride]);
		    tmp11 = c_im(inout[4 * iostride]);
		    tmp8 = c_re(W[3]);
		    tmp10 = c_im(W[3]);
		    tmp12 = (tmp8 * tmp9) + (tmp10 * tmp11);
		    tmp92 = (tmp8 * tmp11) - (tmp10 * tmp9);
	       }
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[12 * iostride]);
		    tmp16 = c_im(inout[12 * iostride]);
		    tmp13 = c_re(W[11]);
		    tmp15 = c_im(W[11]);
		    tmp17 = (tmp13 * tmp14) + (tmp15 * tmp16);
		    tmp93 = (tmp13 * tmp16) - (tmp15 * tmp14);
	       }
	       tmp18 = tmp12 + tmp17;
	       tmp193 = tmp12 - tmp17;
	       tmp94 = tmp92 - tmp93;
	       tmp177 = tmp92 + tmp93;
	  }
	  {
	       fftw_real tmp71;
	       fftw_real tmp108;
	       fftw_real tmp87;
	       fftw_real tmp115;
	       fftw_real tmp76;
	       fftw_real tmp109;
	       fftw_real tmp82;
	       fftw_real tmp114;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp68;
		    fftw_real tmp70;
		    fftw_real tmp67;
		    fftw_real tmp69;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp68 = c_re(inout[15 * iostride]);
		    tmp70 = c_im(inout[15 * iostride]);
		    tmp67 = c_re(W[14]);
		    tmp69 = c_im(W[14]);
		    tmp71 = (tmp67 * tmp68) + (tmp69 * tmp70);
		    tmp108 = (tmp67 * tmp70) - (tmp69 * tmp68);
	       }
	       {
		    fftw_real tmp84;
		    fftw_real tmp86;
		    fftw_real tmp83;
		    fftw_real tmp85;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp84 = c_re(inout[11 * iostride]);
		    tmp86 = c_im(inout[11 * iostride]);
		    tmp83 = c_re(W[10]);
		    tmp85 = c_im(W[10]);
		    tmp87 = (tmp83 * tmp84) + (tmp85 * tmp86);
		    tmp115 = (tmp83 * tmp86) - (tmp85 * tmp84);
	       }
	       {
		    fftw_real tmp73;
		    fftw_real tmp75;
		    fftw_real tmp72;
		    fftw_real tmp74;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp73 = c_re(inout[7 * iostride]);
		    tmp75 = c_im(inout[7 * iostride]);
		    tmp72 = c_re(W[6]);
		    tmp74 = c_im(W[6]);
		    tmp76 = (tmp72 * tmp73) + (tmp74 * tmp75);
		    tmp109 = (tmp72 * tmp75) - (tmp74 * tmp73);
	       }
	       {
		    fftw_real tmp79;
		    fftw_real tmp81;
		    fftw_real tmp78;
		    fftw_real tmp80;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp79 = c_re(inout[3 * iostride]);
		    tmp81 = c_im(inout[3 * iostride]);
		    tmp78 = c_re(W[2]);
		    tmp80 = c_im(W[2]);
		    tmp82 = (tmp78 * tmp79) + (tmp80 * tmp81);
		    tmp114 = (tmp78 * tmp81) - (tmp80 * tmp79);
	       }
	       {
		    fftw_real tmp113;
		    fftw_real tmp116;
		    fftw_real tmp110;
		    fftw_real tmp111;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp77 = tmp71 + tmp76;
		    tmp88 = tmp82 + tmp87;
		    tmp161 = tmp77 - tmp88;
		    tmp113 = tmp71 - tmp76;
		    tmp116 = tmp114 - tmp115;
		    tmp117 = tmp113 - tmp116;
		    tmp141 = tmp113 + tmp116;
		    tmp162 = tmp108 + tmp109;
		    tmp163 = tmp114 + tmp115;
		    tmp164 = tmp162 - tmp163;
		    tmp110 = tmp108 - tmp109;
		    tmp111 = tmp82 - tmp87;
		    tmp112 = tmp110 + tmp111;
		    tmp140 = tmp110 - tmp111;
	       }
	  }
	  {
	       fftw_real tmp24;
	       fftw_real tmp97;
	       fftw_real tmp29;
	       fftw_real tmp98;
	       fftw_real tmp96;
	       fftw_real tmp99;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp21;
		    fftw_real tmp23;
		    fftw_real tmp20;
		    fftw_real tmp22;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp21 = c_re(inout[2 * iostride]);
		    tmp23 = c_im(inout[2 * iostride]);
		    tmp20 = c_re(W[1]);
		    tmp22 = c_im(W[1]);
		    tmp24 = (tmp20 * tmp21) + (tmp22 * tmp23);
		    tmp97 = (tmp20 * tmp23) - (tmp22 * tmp21);
	       }
	       {
		    fftw_real tmp26;
		    fftw_real tmp28;
		    fftw_real tmp25;
		    fftw_real tmp27;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp26 = c_re(inout[10 * iostride]);
		    tmp28 = c_im(inout[10 * iostride]);
		    tmp25 = c_re(W[9]);
		    tmp27 = c_im(W[9]);
		    tmp29 = (tmp25 * tmp26) + (tmp27 * tmp28);
		    tmp98 = (tmp25 * tmp28) - (tmp27 * tmp26);
	       }
	       tmp30 = tmp24 + tmp29;
	       tmp153 = tmp97 + tmp98;
	       tmp96 = tmp24 - tmp29;
	       tmp99 = tmp97 - tmp98;
	       tmp100 = tmp96 - tmp99;
	       tmp137 = tmp96 + tmp99;
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp102;
	       fftw_real tmp40;
	       fftw_real tmp103;
	       fftw_real tmp101;
	       fftw_real tmp104;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[14 * iostride]);
		    tmp34 = c_im(inout[14 * iostride]);
		    tmp31 = c_re(W[13]);
		    tmp33 = c_im(W[13]);
		    tmp35 = (tmp31 * tmp32) + (tmp33 * tmp34);
		    tmp102 = (tmp31 * tmp34) - (tmp33 * tmp32);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[6 * iostride]);
		    tmp39 = c_im(inout[6 * iostride]);
		    tmp36 = c_re(W[5]);
		    tmp38 = c_im(W[5]);
		    tmp40 = (tmp36 * tmp37) + (tmp38 * tmp39);
		    tmp103 = (tmp36 * tmp39) - (tmp38 * tmp37);
	       }
	       tmp41 = tmp35 + tmp40;
	       tmp152 = tmp102 + tmp103;
	       tmp101 = tmp35 - tmp40;
	       tmp104 = tmp102 - tmp103;
	       tmp105 = tmp101 + tmp104;
	       tmp136 = tmp104 - tmp101;
	  }
	  {
	       fftw_real tmp48;
	       fftw_real tmp119;
	       fftw_real tmp64;
	       fftw_real tmp126;
	       fftw_real tmp53;
	       fftw_real tmp120;
	       fftw_real tmp59;
	       fftw_real tmp125;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp45;
		    fftw_real tmp47;
		    fftw_real tmp44;
		    fftw_real tmp46;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp45 = c_re(inout[iostride]);
		    tmp47 = c_im(inout[iostride]);
		    tmp44 = c_re(W[0]);
		    tmp46 = c_im(W[0]);
		    tmp48 = (tmp44 * tmp45) + (tmp46 * tmp47);
		    tmp119 = (tmp44 * tmp47) - (tmp46 * tmp45);
	       }
	       {
		    fftw_real tmp61;
		    fftw_real tmp63;
		    fftw_real tmp60;
		    fftw_real tmp62;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp61 = c_re(inout[13 * iostride]);
		    tmp63 = c_im(inout[13 * iostride]);
		    tmp60 = c_re(W[12]);
		    tmp62 = c_im(W[12]);
		    tmp64 = (tmp60 * tmp61) + (tmp62 * tmp63);
		    tmp126 = (tmp60 * tmp63) - (tmp62 * tmp61);
	       }
	       {
		    fftw_real tmp50;
		    fftw_real tmp52;
		    fftw_real tmp49;
		    fftw_real tmp51;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp50 = c_re(inout[9 * iostride]);
		    tmp52 = c_im(inout[9 * iostride]);
		    tmp49 = c_re(W[8]);
		    tmp51 = c_im(W[8]);
		    tmp53 = (tmp49 * tmp50) + (tmp51 * tmp52);
		    tmp120 = (tmp49 * tmp52) - (tmp51 * tmp50);
	       }
	       {
		    fftw_real tmp56;
		    fftw_real tmp58;
		    fftw_real tmp55;
		    fftw_real tmp57;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp56 = c_re(inout[5 * iostride]);
		    tmp58 = c_im(inout[5 * iostride]);
		    tmp55 = c_re(W[4]);
		    tmp57 = c_im(W[4]);
		    tmp59 = (tmp55 * tmp56) + (tmp57 * tmp58);
		    tmp125 = (tmp55 * tmp58) - (tmp57 * tmp56);
	       }
	       {
		    fftw_real tmp124;
		    fftw_real tmp127;
		    fftw_real tmp121;
		    fftw_real tmp122;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp54 = tmp48 + tmp53;
		    tmp65 = tmp59 + tmp64;
		    tmp156 = tmp54 - tmp65;
		    tmp124 = tmp48 - tmp53;
		    tmp127 = tmp125 - tmp126;
		    tmp128 = tmp124 - tmp127;
		    tmp144 = tmp124 + tmp127;
		    tmp157 = tmp119 + tmp120;
		    tmp158 = tmp125 + tmp126;
		    tmp159 = tmp157 - tmp158;
		    tmp121 = tmp119 - tmp120;
		    tmp122 = tmp59 - tmp64;
		    tmp123 = tmp121 + tmp122;
		    tmp143 = tmp121 - tmp122;
	       }
	  }
	  {
	       fftw_real tmp107;
	       fftw_real tmp131;
	       fftw_real tmp196;
	       fftw_real tmp198;
	       fftw_real tmp130;
	       fftw_real tmp191;
	       fftw_real tmp134;
	       fftw_real tmp197;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp95;
		    fftw_real tmp106;
		    fftw_real tmp192;
		    fftw_real tmp195;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp95 = tmp91 - tmp94;
		    tmp106 = K707106781 * (tmp100 + tmp105);
		    tmp107 = tmp95 - tmp106;
		    tmp131 = tmp95 + tmp106;
		    tmp192 = K707106781 * (tmp137 + tmp136);
		    tmp195 = tmp193 + tmp194;
		    tmp196 = tmp192 + tmp195;
		    tmp198 = tmp195 - tmp192;
	       }
	       {
		    fftw_real tmp118;
		    fftw_real tmp129;
		    fftw_real tmp132;
		    fftw_real tmp133;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp118 = (K923879532 * tmp112) - (K382683432 * tmp117);
		    tmp129 = (K923879532 * tmp123) + (K382683432 * tmp128);
		    tmp130 = tmp118 - tmp129;
		    tmp191 = tmp129 + tmp118;
		    tmp132 = (K923879532 * tmp128) - (K382683432 * tmp123);
		    tmp133 = (K382683432 * tmp112) + (K923879532 * tmp117);
		    tmp134 = tmp132 + tmp133;
		    tmp197 = tmp132 - tmp133;
	       }
	       c_re(inout[13 * iostride]) = tmp107 - tmp130;
	       c_re(inout[5 * iostride]) = tmp107 + tmp130;
	       c_re(inout[9 * iostride]) = tmp131 - tmp134;
	       c_re(inout[iostride]) = tmp131 + tmp134;
	       c_im(inout[iostride]) = tmp191 + tmp196;
	       c_im(inout[9 * iostride]) = tmp196 - tmp191;
	       c_im(inout[5 * iostride]) = tmp197 + tmp198;
	       c_im(inout[13 * iostride]) = tmp198 - tmp197;
	  }
	  {
	       fftw_real tmp139;
	       fftw_real tmp147;
	       fftw_real tmp202;
	       fftw_real tmp204;
	       fftw_real tmp146;
	       fftw_real tmp199;
	       fftw_real tmp150;
	       fftw_real tmp203;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp135;
		    fftw_real tmp138;
		    fftw_real tmp200;
		    fftw_real tmp201;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp135 = tmp91 + tmp94;
		    tmp138 = K707106781 * (tmp136 - tmp137);
		    tmp139 = tmp135 - tmp138;
		    tmp147 = tmp135 + tmp138;
		    tmp200 = K707106781 * (tmp100 - tmp105);
		    tmp201 = tmp194 - tmp193;
		    tmp202 = tmp200 + tmp201;
		    tmp204 = tmp201 - tmp200;
	       }
	       {
		    fftw_real tmp142;
		    fftw_real tmp145;
		    fftw_real tmp148;
		    fftw_real tmp149;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp142 = (K382683432 * tmp140) - (K923879532 * tmp141);
		    tmp145 = (K382683432 * tmp143) + (K923879532 * tmp144);
		    tmp146 = tmp142 - tmp145;
		    tmp199 = tmp145 + tmp142;
		    tmp148 = (K382683432 * tmp144) - (K923879532 * tmp143);
		    tmp149 = (K923879532 * tmp140) + (K382683432 * tmp141);
		    tmp150 = tmp148 + tmp149;
		    tmp203 = tmp148 - tmp149;
	       }
	       c_re(inout[15 * iostride]) = tmp139 - tmp146;
	       c_re(inout[7 * iostride]) = tmp139 + tmp146;
	       c_re(inout[11 * iostride]) = tmp147 - tmp150;
	       c_re(inout[3 * iostride]) = tmp147 + tmp150;
	       c_im(inout[3 * iostride]) = tmp199 + tmp202;
	       c_im(inout[11 * iostride]) = tmp202 - tmp199;
	       c_im(inout[7 * iostride]) = tmp203 + tmp204;
	       c_im(inout[15 * iostride]) = tmp204 - tmp203;
	  }
	  {
	       fftw_real tmp155;
	       fftw_real tmp167;
	       fftw_real tmp188;
	       fftw_real tmp190;
	       fftw_real tmp166;
	       fftw_real tmp189;
	       fftw_real tmp170;
	       fftw_real tmp185;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp151;
		    fftw_real tmp154;
		    fftw_real tmp186;
		    fftw_real tmp187;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp151 = tmp7 - tmp18;
		    tmp154 = tmp152 - tmp153;
		    tmp155 = tmp151 + tmp154;
		    tmp167 = tmp151 - tmp154;
		    tmp186 = tmp30 - tmp41;
		    tmp187 = tmp180 - tmp177;
		    tmp188 = tmp186 + tmp187;
		    tmp190 = tmp187 - tmp186;
	       }
	       {
		    fftw_real tmp160;
		    fftw_real tmp165;
		    fftw_real tmp168;
		    fftw_real tmp169;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp160 = tmp156 - tmp159;
		    tmp165 = tmp161 + tmp164;
		    tmp166 = K707106781 * (tmp160 + tmp165);
		    tmp189 = K707106781 * (tmp160 - tmp165);
		    tmp168 = tmp164 - tmp161;
		    tmp169 = tmp156 + tmp159;
		    tmp170 = K707106781 * (tmp168 - tmp169);
		    tmp185 = K707106781 * (tmp169 + tmp168);
	       }
	       c_re(inout[10 * iostride]) = tmp155 - tmp166;
	       c_re(inout[2 * iostride]) = tmp155 + tmp166;
	       c_re(inout[14 * iostride]) = tmp167 - tmp170;
	       c_re(inout[6 * iostride]) = tmp167 + tmp170;
	       c_im(inout[2 * iostride]) = tmp185 + tmp188;
	       c_im(inout[10 * iostride]) = tmp188 - tmp185;
	       c_im(inout[6 * iostride]) = tmp189 + tmp190;
	       c_im(inout[14 * iostride]) = tmp190 - tmp189;
	  }
	  {
	       fftw_real tmp43;
	       fftw_real tmp171;
	       fftw_real tmp182;
	       fftw_real tmp184;
	       fftw_real tmp90;
	       fftw_real tmp183;
	       fftw_real tmp174;
	       fftw_real tmp175;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp19;
		    fftw_real tmp42;
		    fftw_real tmp176;
		    fftw_real tmp181;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp19 = tmp7 + tmp18;
		    tmp42 = tmp30 + tmp41;
		    tmp43 = tmp19 + tmp42;
		    tmp171 = tmp19 - tmp42;
		    tmp176 = tmp153 + tmp152;
		    tmp181 = tmp177 + tmp180;
		    tmp182 = tmp176 + tmp181;
		    tmp184 = tmp181 - tmp176;
	       }
	       {
		    fftw_real tmp66;
		    fftw_real tmp89;
		    fftw_real tmp172;
		    fftw_real tmp173;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp66 = tmp54 + tmp65;
		    tmp89 = tmp77 + tmp88;
		    tmp90 = tmp66 + tmp89;
		    tmp183 = tmp66 - tmp89;
		    tmp172 = tmp162 + tmp163;
		    tmp173 = tmp157 + tmp158;
		    tmp174 = tmp172 - tmp173;
		    tmp175 = tmp173 + tmp172;
	       }
	       c_re(inout[8 * iostride]) = tmp43 - tmp90;
	       c_re(inout[0]) = tmp43 + tmp90;
	       c_re(inout[12 * iostride]) = tmp171 - tmp174;
	       c_re(inout[4 * iostride]) = tmp171 + tmp174;
	       c_im(inout[0]) = tmp175 + tmp182;
	       c_im(inout[8 * iostride]) = tmp182 - tmp175;
	       c_im(inout[4 * iostride]) = tmp183 + tmp184;
	       c_im(inout[12 * iostride]) = tmp184 - tmp183;
	  }
     }
}

static const int twiddle_order[] =
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
fftw_codelet_desc fftwi_twiddle_16_desc =
{
     "fftwi_twiddle_16",
     (void (*)()) fftwi_twiddle_16,
     16,
     FFTW_BACKWARD,
     FFTW_TWIDDLE,
     363,
     15,
     twiddle_order,
};
