// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include "SndWaveX.h"

SndWaveX::SndWaveX(char* name, short mode, 
          short channels, int channelmask, short bits, short format,
             SndObj** inputlist, float spos, 
			 int vecsize, float sr):
SndWave(name,mode,channels, bits,inputlist,spos,vecsize,sr){

   short cbsize;
   m_len = 40;
   m_format = WAVE_FORMAT_EXTENSIBLE;
   m_hdrsize = 60;
   m_subformat = format;
   m_fp = (float *)m_buffer;
   m_dp = (double *)m_buffer;
   cbsize = lenshort((short)22);
    if(mode != READ){
   GUID subfmt;
   short sbits;
   int mask;
   
   PutHeader(0,m_hdrsize,m_len, m_format);
   sbits = lenshort((short)m_bits);
   mask = lenlong((long) (m_ChannelMask = channelmask));
   subfmt.Data1 = lenlong((long)format);
   subfmt.Data2 = lenshort((short)0x0000);
   subfmt.Data3 = lenshort((short)0x0010); 
   subfmt.Data4[0] = 0x80;
   subfmt.Data4[1] =0x00;
	   subfmt.Data4[2] = 0x00;
	   subfmt.Data4[3] = 0xaa;
	   subfmt.Data4[4] = 0x00;
	   subfmt.Data4[5] = 0x38;
	   subfmt.Data4[6] = 0x9b;
	   subfmt.Data4[7] = 0x71;
   m_SubFormat = subfmt;

	if(mode != APPEND){
   fseek(m_file, sizeof(wave_head), SEEK_SET); 
   fwrite(&cbsize, sizeof(short), 1, m_file);
   fwrite(&sbits, sizeof(short), 1, m_file);
   fwrite(&mask, sizeof(int), 1, m_file);
   fwrite(&subfmt, sizeof(GUID), 1, m_file);
   m_wchkpos = ftell(m_file);
   fwrite(&m_wdata, sizeof(wave_data), 1, m_file);
   m_datapos = ftell(m_file);
    if(spos > 0) SetPos(spos);     
		} else m_wchkpos = sizeof(wave_head) + 22 + 2;
    
	}
     
	else {
    // if this is a format extensible   
    if(m_header.format == -2){
       fseek(m_file, sizeof(wave_head), SEEK_SET);	   
	   fread(&cbsize,sizeof(short),1, m_file);
	   cbsize = natlshort((short)cbsize); 
       fread(&m_bits,sizeof(short),1, m_file);
	   m_bits = natlshort((short)m_bits);
	   fread(&m_ChannelMask,sizeof(int),1, m_file);
       m_ChannelMask = natllong((long) m_ChannelMask);
	   fread(&m_SubFormat, sizeof(GUID), 1, m_file);
       m_subformat = m_SubFormat.Data1 = natllong((long) m_SubFormat.Data1 );
	   m_SubFormat.Data2 = natlshort((short) m_SubFormat.Data2 );
	   m_SubFormat.Data3 = natlshort((short) m_SubFormat.Data3 );
	   m_iswavex = true;	  
	} else m_iswavex = false;

	fseek(m_file, m_datapos, SEEK_SET);
    if(spos > 0) SetPos(spos);
	}

}
SndWaveX::~SndWaveX(){

}

void
SndWaveX::GetHeader(WAVEFORMATEXTENSIBLE* pheader){
pheader->waveformatex.len0 = m_header.len0; 
pheader->waveformatex.len = m_header.len;  
pheader->waveformatex.format = m_header.format; // PCM == 1
pheader->waveformatex.nchns = m_header.nchns;
pheader->waveformatex.rate = m_header.rate; // sampling rate
pheader->waveformatex.aver = m_header.aver;// bytes per sec
pheader->waveformatex.nBlockAlign = m_header.nBlockAlign; // bytes per frame  
pheader->waveformatex.size = m_header.size;	// bits per sample 
pheader->waveformat_ext.wValidBitsPerSample = m_bits;
pheader->waveformat_ext.dwChannelMask = m_ChannelMask;
pheader->waveformat_ext.SubFormat =  m_SubFormat;
}


short 
SndWaveX::Read(){

if(!m_error && (m_mode == READ) && !feof(m_file)){
 
 int i;
 short items;
 items = fread(m_buffer, 1, m_buffsize, m_file);
  items = (items < m_buffsize ? m_itemsleft : items/m_sampsize);


 switch(m_bits) {
  
  case 16:
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
	for(i=0; i < m_channels; i++)
    m_output[m_vecpos+i] = (m_vecpos+i < items ?
	                     (float) natlshort(m_sp[m_vecpos+i])
	                             :  0.f);

  break;

  case 8: 
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
       for(i=0; i < m_channels; i++)
  m_output[m_vecpos+i] = (m_vecpos+i < items ?
                            (float) m_cp[m_vecpos+i] 
                            :  0.f);
  break;

  case 32:
  if(m_subformat == PCM)
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
   for(i=0; i < m_channels; i++)
	   m_output[m_vecpos+i] = (m_vecpos+i < items ?
	                              (float) natllong(m_lp[m_vecpos+i])
								  : 0.f);
   else
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
   for(i=0; i < m_channels; i++)
	   m_output[m_vecpos+i] = (m_vecpos+i < items ?
	                              m_fp[m_vecpos+i]
								  : 0.f);
  case 24:
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
   for(i=0; i < m_channels; i++)
	   m_output[m_vecpos+i] = (m_vecpos+i < items ?
	                              (float) natllong(
								  (*(long *)m_s24p[m_vecpos+i].s))
								  : 0.f);	
  case 64:
  for(m_vecpos=0; m_vecpos < m_samples; m_vecpos+=m_channels)
   for(i=0; i < m_channels; i++)
	   m_output[m_vecpos+i] = (float) (m_vecpos+i < items ?
	                                    m_dp[m_vecpos+i]
								          : 0.);
  break;

 }
 return items;
}

for(m_vecpos=0; m_vecpos < m_samples; m_vecpos++)
        m_output[m_vecpos] = 0.f;
return 0;


}



short
SndWaveX::Write(){
if(!m_error && (m_mode != READ)){
 int i,n;
  switch(m_bits){
  case 16:
  for(m_vecpos=n=0; m_vecpos < m_samples; m_vecpos+=m_channels, n++)
	for(i = 0; i < m_channels; i++)
			if(m_IOobjs[i])
         m_sp[m_vecpos+i] = lenshort((short)m_IOobjs[i]->Output(n));
  return (short) fwrite(m_sp, m_buffsize, 1, m_file);
  case 8:
  for(m_vecpos=n=0; m_vecpos < m_samples; m_vecpos+=m_channels, n++)
    for(i = 0; i < m_channels; i++)
		 	if(m_IOobjs[i])
         m_cp[m_vecpos+i] = (char) m_IOobjs[i]->Output(n);
  return (short) fwrite(m_cp, m_buffsize, 1, m_file);
  case 32:
  for(m_vecpos=n=0; m_vecpos < m_samples; m_vecpos+=m_channels, n++)
    for(i = 0; i < m_channels; i++)
		if(m_IOobjs[i]){
		if(m_subformat == PCM)
         m_lp[m_vecpos+i] = lenlong((long) m_IOobjs[i]->Output(n));
		else
         m_fp[m_vecpos+i] = m_IOobjs[i]->Output(n);
		}
  return (short) fwrite(m_buffer, m_buffsize, 1, m_file);
  
  case 24:
    for(m_vecpos=n=0; m_vecpos < m_samples; m_vecpos+=m_channels, n++)
    for(i = 0; i < m_channels; i++)
		if(m_IOobjs[i]){ 
		 (*(long *)m_s24p[m_vecpos+i].s) = lenlong((long) m_IOobjs[i]->Output(n));
		}
  return (short) fwrite(m_s24p, m_buffsize, 1, m_file);

  case 64:
    for(m_vecpos=n=0; m_vecpos < m_samples; m_vecpos+=m_channels, n++)
    for(i = 0; i < m_channels; i++)
		 	if(m_IOobjs[i])
         m_dp[m_vecpos+i] = (double) m_IOobjs[i]->Output(n);
  return (short) fwrite(m_buffer, m_buffsize, 1, m_file);
  }
 }
 return 0;
}

