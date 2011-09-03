#include "daub.h"
#include <iostream>
using namespace std;

#define NR_END 1
#define FREE_ARG char*

float *vector(long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh] */
{
	float *v;

	v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
	if (!v) printf("allocation failure in vector()");
	return v-nl+NR_END;
}

void free_vector(float *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void wt1(float a[], unsigned long n, int isign,
	void (*wtstep)(float [], unsigned long, int))
{
	unsigned long nn;
	//cerr << "wt1, n = " << n << endl;
	if (n < 2) return; // changed 4's to 2's to get single S and D coefficient at coarsest resolution
	if (isign >= 0) {
		for (nn=n;nn>=2;nn>>=1) (*wtstep)(a,nn,isign);
	} else {
		for (nn=2;nn<=n;nn<<=1) (*wtstep)(a,nn,isign);
	}
}
/* (C) Copr. 1986-92 Numerical Recipes Software !'K4$<%#110L&")|oV'4. */

#define NRANSI	
#define C0 0.4829629131445341	
#define C1 0.8365163037378079	
#define C2 0.2241438680420134
#define C3 -0.1294095225512604

void daub4(float a[], unsigned long n, int isign)
{
	float *wksp;
	unsigned long nh,nh1,i,j;

	if (n < 2) return;	
	wksp=vector(1,n);
	nh1=(nh=n >> 1)+1;
	if (isign >= 0) {
		for (i=1,j=1;j<=n-3;j+=2,i++) {	
			wksp[i]=C0*a[j]+C1*a[j+1]+C2*a[j+2]+C3*a[j+3];
			wksp[i+nh] = C3*a[j]-C2*a[j+1]+C1*a[j+2]-C0*a[j+3];
		}
		wksp[i]=C0*a[n-1]+C1*a[n]+C2*a[1]+C3*a[2];
		wksp[i+nh] = C3*a[n-1]-C2*a[n]+C1*a[1]-C0*a[2];
	} else {
		wksp[1]=C2*a[nh]+C1*a[n]+C0*a[1]+C3*a[nh1];
		wksp[2] = C3*a[nh]-C0*a[n]+C1*a[1]-C2*a[nh1];
		for (i=1,j=3;i<nh;i++) {	
			wksp[j++]=C2*a[i]+C1*a[i+nh]+C0*a[i+1]+C3*a[i+nh1];
			wksp[j++] = C3*a[i]-C0*a[i+nh]+C1*a[i+1]-C2*a[i+nh1];
		}
	}
	for (i=1;i<=n;i++) a[i]=wksp[i];
	free_vector(wksp,1,n);
}
#undef C0
#undef C1
#undef C2
#undef C3
#undef NRANSI


/* More general setting for larger filters, from same book p. 596 - 597 */
typedef struct {
	int ncof, ioff, joff;
	float *cc, *cr;
} wavefilt;

wavefilt wfilt;

void pwtset( int n )
{
	// Initializing routine for pwt, must be called once before the first use of pwt.
	// Slower than daub4
	// n is the filter length (4 for same results as daub4, 10 for 'db5' (daub10, 5 vanishing moments))

	int k;
	float sig = -1.0;
	//static float c4[5] = {0.0, -0.1294095225512604, 0.2241438680420134, 0.8365163037378079,  
	//	0.4829629131445341}; 
	static float c4[5] = {0.0, 0.4829629131445341, 0.8365163037378079, 0.2241438680420134, 
		-0.1294095225512604};
    //static float c10[11] = {0.0, 0.00333572528500, -0.01258075199902, -0.00624149021301, 0.07757149384007,
	//	-0.03224486958503, -0.24229488706619, 0.13842814590110, 0.72430852843857, 0.60382926979747,
	//	0.16010239797413};
	static float c10[11] = {0.0, 0.16010239797420, 0.60382926979719, 0.72430852843777, 
		0.13842814590132, -0.24229488706638, -0.03224486958463, 0.07757149384004, -0.00624149021279, 
		-0.01258075199908, 0.00333572528547};
	static float c12[13] = {0.0, 0.111540743350, 0.494623890398, 0.751133908021, 0.315250351709, 
		-0.226264693965, -0.129766867567, 0.097501605587, 0.027522865530, -0.031582039318, 
		0.000553842201, 0.004777257511, -0.001077301085};
	static float c4r[5], c10r[11], c12r[13];

	wfilt.ncof = n;
	if( n == 4 ) {
		wfilt.cc = c4;
		wfilt.cr = c4r;
	}
	else if( n == 10 ) {
		wfilt.cc = c10;
		wfilt.cr = c10r;
	}
	else if( n == 12 ) {
		wfilt.cc = c12;
		wfilt.cr = c12r;
	}
	else {
		printf( "unimplemented value n in pwtset\n" );
		exit(1);
	}

	for( k = 1; k <= n; k++ ) {
		wfilt.cr[wfilt.ncof + 1 - k] = sig * wfilt.cc[k];
		sig = -sig;
	}

	wfilt.ioff = wfilt.joff = -(n >> 1);
	
	// These values center the "support" of the wavelets at each level. Alternatively, the "peaks"
	// of the wavelets can be approximately centered by the choices ioff = -2 and joff = -n+2
}


// The following can be an instance of wtstep, used after pwtset
void pwt( float a[], unsigned long n, int isign )
{
	// Partial wavelet transform, applies arbitrary wavelet filter to data vector a[1..n]
	// if isign = 1, and transpose / inverse if isign == -1
	//cerr << "I'm pwt!! n = " << n << endl;
	float ai, ai1, *wksp;
	unsigned long i, ii, j, jf, jr, k, n1, ni, nj, nh, nmod;

	if( n < 2 ) return;			// decomposes till there is one S coefficient
	wksp = vector(1,n);
	nmod = wfilt.ncof * n;		// A positive constant equal to zero mod n
	n1 = n - 1;					// Mask of all bits, since n is a power of 2
	nh = n >> 1;
	for( j = 1; j <= n; j++ )
		wksp[j] = 0.0;

	if( isign >= 0 ) {			// Apply filter
		for( ii = 1, i = 1; i <= n; i += 2, ii++ ) {
			ni = i + nmod + wfilt.ioff;		// Pointer to be incremented and wrapped around
			nj = i + nmod + wfilt.joff;
			for( k = 1; k <= wfilt.ncof; k++ ) {
				jf = n1 & (ni + k );	// They use 'bitwise and' to wrap-around the pointers
				jr = n1 & (nj + k );
				wksp[ii] += wfilt.cc[k] * a[jf+1];
				wksp[ii+nh] += wfilt.cr[k] * a[jr+1];
			}
		}
	}
	else {						// Apply transpose filter
		for( ii = 1, i = 1; i <= n; i += 2, ii++ ) {
			ai = a[ii];
			ai1 = a[ii+nh];
			ni = i + nmod + wfilt.ioff;		// Pointer to be incremented and wrapped around
			nj = i + nmod + wfilt.joff;
			for( k = 1; k <= wfilt.ncof; k++ ) {
				jf = (n1 & (ni + k)) + 1;
				jr = (n1 & (nj + k)) + 1;
				wksp[jf] += wfilt.cc[k] * ai;
				wksp[jr] += wfilt.cr[k] * ai1;
			}
		}
	}

	for( j = 1; j <= n; j++ )
		a[j] = wksp[j];		// Copy the results back from workspace
	
	free_vector( wksp, 1, n );

}


/* (C) Copr. 1986-92 Numerical Recipes Software !'K4$<%#110L&")|oV'4. */
