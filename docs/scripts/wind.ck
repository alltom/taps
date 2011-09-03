// noise generator, biquad filter, dac (audio output) 
noise n => biquad f => dac;
// set biquad pole radius
.95 => f.prad;
// set biquad gain
.05 => f.gain;
// set equal zeros 
1 => f.eqzs;
// our float
0.0 => float t;
.50 => n.gain;

// infinite time-loop
while( true )
{
    // sweep the filter resonant frequency
    100.0 + std.abs(math.sin(t)) * 1000.0 => f.pfreq;
    t + .05 => t;
    // advance time
    100::ms => now;
}
