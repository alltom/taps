sinosc s => dac;
.25 => s.gain;

0.0 => float t;

while( true )
{
    (2000.0 + std.abs(3000.0*math.sin(t))) => s.sfreq;
    t + .0024834 => t;
    .5::ms => now;
}
