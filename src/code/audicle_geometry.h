/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis, 
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: audicle_geometry.h
// desc: ...
//
// authors: 
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_GEOMETRY_H__
#define __AUDICLE_GEOMETRY_H__

#include "audicle_def.h"


#if defined ( __PLATFORM_WIN32__ ) 

static t_TAPINT isinf( t_TAPFLOAT d )
{
    int expon = 0;
    t_TAPFLOAT val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 1;
        } else if (val == -0.5) {
            return -1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

static t_TAPINT isnan (t_TAPFLOAT d)
{
    int expon = 0;
    t_TAPFLOAT val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 0;
        } else if (val == -0.5) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

#endif


#define ASSERT_ARRAY 0
#define RAYEPS 1e-5

/* *LEAVE* this as t_TAPFLOAT!*/
typedef t_TAPFLOAT Flt;


/* The Point2D and Point3D classes are just a 2D and 3D vectors with methods 
 * supporting indexing, printing, taking the dot product with another vector, 
 * obtaining length, returning a unit-length scaled version of itself,
 * negation, scaling, adding, subtracting, and doing component-wise 
 * multiplication. Additionally, the 3D vector supports taking the cross-
 * product. */

/* Note that coordinates of the vector are accessed using square brackets in 
 * the same way that an array is so that:
 *   Point3D p(0.0,1.0,2.0);
 *   printf("%f\n",p[1]);
 * should print out "1.0" */

class Point3D;
class Color4D;
class Matrix;

typedef class Point2D Vec2D;

class Point2D{
  Flt p[2];
 public:
  Point2D()             {p[0]=p[1]=0.0;}
  Point2D(Flt x,Flt y)  {p[0]=x; p[1]=y;}
  Point2D(Point3D p3, Matrix * xf = NULL );

  Flt& operator[] (t_TAPINT index);
  const Flt& operator[] (t_TAPINT index) const;
  const t_TAPFLOAT* data() { return p; }  
  t_TAPFLOAT * pdata()      { return p; }
  void print() const {printf("(%f,%f)",p[0],p[1]);}
  void printnl() const {print();printf("\n");}
  bool operator== (Point2D pt) { return (p[0]==pt[0] && p[1]==pt[1] ); } 
  Flt dot(Point2D pt);
  Flt perp(Point2D pt);

  Flt length(void);

  Point2D unit(void);

  Point2D negate(void);
  Point2D operator- (void);

  Point2D scale(Flt scl);
  Point2D operator* (Flt scale);
  Point2D operator/ (Flt scale);

  Point2D add(Point2D pt);
  Point2D operator+ (Point2D pt);

  void  operator +=(Point2D v)  { p[0] += v[0]; p[1] += v[1]    ; }
  void  operator -=(Point2D v)  { p[0] -= v[0]; p[1] -= v[1]    ; }
  void  operator *=(Flt scale)   { p[0] *= scale; p[1] *= scale; }
  void  operator /=(Flt scale)   { p[0] /= scale; p[1] /= scale; }

  Flt normalize(); 
  Point2D subtract(Point2D pt);
  Point2D operator- (Point2D pt);
  Point2D interp ( Point2D pt, t_TAPFLOAT w)     {return scale(1.0-w) + pt.scale(w); } 
  bool    inSeg( Point2D x1, Point2D x2 ) ;
  bool    intersect_rad( Point2D q, t_TAPFLOAT dist, Point2D &out);
  bool    intersect_lines( Point2D a1, Point2D a2, Point2D b1, Point2D b2, Point2D &out );
  t_TAPINT     clock( Point2D a, Point2D c );
  Point2D mult(Point2D pt); //pairwise
  Point2D div(Point2D pt);  //pairwise
};

enum {   SQUASH_IDENT, SQUASH_VOLUME, SQUASH_CONST }; 

typedef class Point3D Vec3D;

class Point3D{
  Flt p[3];
 public:
  Point3D(){p[0]=p[1]=p[2]=0;}
  Point3D(Flt x, Flt y )        {   p[0]=x; p[1]=y; p[2]=0;}
  Point3D(Flt x,Flt y,Flt z)    {   p[0]=x; p[1]=y; p[2]=z;}
  Point3D(Flt* pf)  {   p[0]=pf[0]; p[1]=pf[1]; p[2]=pf[2];}
  Point3D(Point2D pt)   {   p[0]=pt[0]; p[1]=pt[1]; p[2]=0;}
  Point3D(Color4D pt);  
  Flt& operator[] (t_TAPINT index);
  bool operator== (Point3D pt) { return (p[0]==pt[0] && p[1]==pt[1] && p[2] == pt[2] ); } 
  const t_TAPFLOAT* data() { return p; }  
  t_TAPFLOAT * pdata()       { return p; }
  void print(void){printf("(%f,%f,%f)",p[0],p[1],p[2]);}
  void printnl(void){print();printf("\n");}
   
  void fprint(FILE* fp){fprintf( fp, "%f %f %f",p[0],p[1],p[2]);}
  void fprintnl(FILE * fp){fprint(fp);fprintf(fp, "\n");}

  void wrap ( t_TAPFLOAT xmin = -1.0, t_TAPFLOAT xspan = 2.0, t_TAPFLOAT ymin = -1.0, t_TAPFLOAT yspan = 2.0, t_TAPFLOAT zmin = -1.0, t_TAPFLOAT zspan = 2.0 ); 

  Flt dot(Point3D pt);

  Flt len2(void);
  Flt length(void);
  Flt dist(Point3D x) { return (*this - x).length(); }
  bool inSeg2D ( Point3D x1, Point3D x2 ) ;
  static bool isect2d ( Point3D a1, Point3D a2, Point3D b1, Point3D b2, Point3D &out ) ;
  static bool isect2d_rad ( Point3D a2, Point3D b2, t_TAPFLOAT dist, Point3D &out ) ;
  Flt nearest_to_line     (Point3D x1, Point3D dir, Point3D* ret = NULL);
  Flt nearest_to_segment  (Point3D x1, Point3D dir, Point3D* ret = NULL);
  Flt nearest_to_ray      (Point3D x1, Point3D dir, Point3D* ret = NULL);
  Flt perp_xz (Point3D p2);
  Flt dot_xz (Point3D p2);
  Flt normalize();
  Point3D norm();
  Point3D unit(void);

  Point3D negate(void);
  Point3D operator- (void);

  Point3D scale(Flt);
  Point3D operator* (Flt scale);
  Point3D operator/ (Flt scale);
  void   operator *=(Flt scale)   { p[0] *= scale; p[1] *= scale; p[2] *= scale; }
  void    operator /=(Flt scale)   { p[0] /= scale; p[1] /= scale; p[2] /= scale; }

  Point3D add(Point3D pt);
  t_TAPFLOAT  operator* (Point3D pt);
  Point3D operator+ (Point3D pt);

  Point3D subtract(Point3D pt);
  Point3D operator- (Point3D pt);

  void    operator +=(Point3D v)      { p[0] += v[0]; p[1] += v[1]; p[2] += v[2]    ;}
  void    operator -=(Point3D v)      { p[0] -= v[0]; p[1] -= v[1]; p[2] -= v[2]    ;}
  
  void    fillMatrix ( t_TAPFLOAT* m, Point3D fwd, Point3D base, t_TAPINT squash=SQUASH_IDENT);
  Point3D project (Point3D pt );
  Point3D crossProduct(Point3D pt);

  Point3D interp ( Point3D pt, t_TAPFLOAT w)     {return scale(1.0-w) + pt.scale(w); } 

  Point3D mult(Point3D pt);


  Point3D      bezier_interp_pt(Point3D** v1, t_TAPINT n, t_TAPFLOAT u );
  Point3D      bezier_interp_pt_dir(Point3D** v1, t_TAPINT n, t_TAPFLOAT u, Point3D &dir);
  Point3D      spline_interp_pt(Point3D** vl, t_TAPFLOAT * pl, t_TAPFLOAT u);
  Point3D      spline_interp_pt_dir(Point3D** vl, t_TAPFLOAT * pl, t_TAPFLOAT u, Point3D &dir);

};

class Color4D{
  Flt p[4];
 public:
  Color4D(){p[0]=p[1]=p[2]=0;p[3]=1;}
  Color4D(Flt x,Flt y,Flt z){p[0]=x;p[1]=y;p[2]=z;p[3]=1;}
  Color4D(Flt x,Flt y,Flt z, Flt a){p[0]=x;p[1]=y;p[2]=z;p[3]=a;}
  Color4D(unsigned char* px)  {     p[0] = ((t_TAPFLOAT)px[0]) / 255.0; 
                                    p[1] = ((t_TAPFLOAT)px[1]) / 255.0; 
                                    p[2] = ((t_TAPFLOAT)px[2]) / 255.0;
                                    p[3] = 1;                        }
  Color4D(Point3D pt);
  Flt& operator[] (t_TAPINT index);

  const t_TAPFLOAT* data() { return p; }  
  t_TAPFLOAT* pdata() { return p; }  
  void print(void)          {printf("(%f,%f,%f,%f)",p[0],p[1],p[2],p[3]);}
  void printnl(void)           {print();printf("\n");}
  void fprint (FILE* fp)    {fprintf(fp,"%f %f %f %f",p[0], p[1], p[2], p[3] );} 
  void fprintnl(FILE* fp)   {fprint(fp);fprintf(fp, "\n");}
  void fill_float(t_TAPFLOAT *dbg) { dbg[0]=p[0]; dbg[1]=p[1]; dbg[2]=p[2]; dbg[3]=p[3];  }
  void fill_ubyte(unsigned char *dbg) {   dbg[0]=(unsigned char) ( x_min(1.0, x_max(0,p[0]))*255 ); 
                                          dbg[1]=(unsigned char) ( x_min(1.0, x_max(0,p[1]))*255 ); 
                                          dbg[2]=(unsigned char) ( x_min(1.0, x_max(0,p[2]))*255 );  }

  Flt dot(Color4D pt);

  Flt length(void);

  Color4D unit(void);

  Color4D negate(void);
  Color4D operator- (void);

  Color4D scale(Flt);
  Color4D operator* (Flt scale);
  Color4D operator/ (Flt scale);

  Color4D add(Color4D pt);
  Color4D operator + (Color4D pt);
  void    operator +=(Color4D pt)     { p[0] += pt[0]; p[1] += pt[1]; p[2] += pt[2]; p[3] += pt[3];}

  Color4D subtract(Color4D pt);
  Color4D operator- (Color4D pt);

  Color4D crossProduct(Color4D pt);
  Color4D interp ( Color4D pt, t_TAPFLOAT w)     {return scale(1.0-w) + pt.scale(w); }

  Color4D mult(Color4D pt);
  Color4D alpha(t_TAPFLOAT a);
};

typedef class Quaternion Quat;
class Quaternion { 
protected:
   
   //  i*i = j*j = k*k = -1;
   //  i*j = k = -j*i;
   //  j*k = i = -k*j;
   //  k*i = j = -i*k;

   t_TAPFLOAT _r;  //real

   Point3D _im;

public:

   Quaternion(Point3D axis , t_TAPFLOAT angle ) {
                                           _r = cos ( angle/2.0 );
                                           _im = axis.norm() * sin ( angle/2.0 );
                                          }
   Quaternion(t_TAPFLOAT w, t_TAPFLOAT x, t_TAPFLOAT y, t_TAPFLOAT z) { _r = w; _im[0]=x; _im[1]=y; _im[2]=z;}
   Quaternion(t_TAPFLOAT w, Point3D im) : _r (w), _im(im) {}
   Quaternion() { _r = 1; _im = Point3D(0,0,0);}

   void print() { printf("%lf, %lf i, %lf j, %lf k", _r, _im[0], _im[1], _im[2]); }
   void printnl() { print(); printf("\n"); }
   
   t_TAPFLOAT      r()  {return _r;}
   Point3D     im() {return _im;}
   Quaternion scale(t_TAPFLOAT d)   { return Quaternion( _r * d , _im * d ); }
   Quaternion negate()  { return Quaternion(-_r, -_im); } 
   Quaternion conj()    { return Quaternion( _r, -_im); }
   Quaternion inv()     { return conj().scale(1.0 / dot(conj()) );} 
   t_TAPFLOAT     mag()     { return sqrt(dot(conj()));}
   Quaternion unit()    { return (mag() > 0 ) ? scale(1.0 / mag()) : Quaternion(1,0,0,0); } 

   t_TAPFLOAT     dot(Quaternion p) { return _r * p._r - (_im.dot(p._im)); }
   Quaternion add(Quaternion p) { return Quaternion ( _r+p._r ,  _im + p._im ); }
   Quaternion prod(Quaternion p) { //  *this x p
                                    return Quaternion ( dot(p), p._im*_r + _im*p._r + _im.crossProduct(p._im)); 
                                 }

   void fprint( FILE * fp)       { fprintf( fp, "%f %f %f %f",_r, _im[0],_im[1],_im[2]); }

   t_TAPFLOAT*    pr()               { return &_r;}
   t_TAPFLOAT*    pim()              { return _im.pdata(); } 

   Point3D pointmul(Point3D p) { 
                                   Quaternion r = Quaternion( 0, p );
                                   return prod( r.prod(inv()) )._im;
                               }

   Quaternion interp(Quaternion r, t_TAPFLOAT w) { return scale(1-w).add(r.scale(w)); }
   Quaternion slerp(Quaternion r, t_TAPFLOAT w) { return interp(r,w).unit(); }

   void test() {
      Point3D x(1,0,0);
      Point3D y(0,1,0);
      Point3D z(0,0,1);

      Point3D px = pointmul(x);
      Point3D py = pointmul(y);
      Point3D pz = pointmul(z);
      printf("Quaternion :");
      printnl();
      printf("test unit axis\n");
      x.print(); printf(" to "); px.printnl();
      y.print(); printf(" to "); py.printnl();
      z.print(); printf(" to "); pz.printnl();
      printf("end unit axis\n");
   } 

   void genMatrix(t_TAPFLOAT *m) { 
      
   }//fill a rotation matrix
};

class BBox { 
protected:
   bool    _hasContent;
   Point3D _min;
   Point3D _max;
public:
   BBox() { reset(); }
   BBox(Point3D p) { set(p); }
   BBox(Point3D p1, Point3D p2) { set(p1,p2); }
   void reset() { _hasContent = false; } 
   void set(Point3D p) { _hasContent=true; _min = p; _max = p;} 
   void set(Point3D p1, Point3D p2) { set(p1); add(p2); } 
   void add(Point3D p) {
      if ( !_hasContent ) set(p);
      else { 
         for ( t_TAPINT i = 0 ; i < 3 ; i++ ) { 
            _min[i] = x_min( p[i], _min[i] );
            _max[i] = x_max( p[i], _max[i] );
         }   
      }
   }
   void add2d(Point2D p ) { //let's be picky!
      if ( !_hasContent ) set(p);
      else { 
         for ( t_TAPINT i = 0 ; i < 2 ; i++ ) { 
            _min[i] = x_min( p[i], _min[i] );
            _max[i] = x_max( p[i], _max[i] );
         }   
      }
   }
   bool     is_set()    { return _hasContent;   } 
   Point3D& pmin()      { return _min;          } 
   Point3D& pmax()      { return _max;          } 
   // is p in the box? that is the question.
   bool in(Point2D p) {
      if( !_hasContent ) return false;
      bool isin = true;
      for( t_TAPINT i = 0; i < 2; i++ )
         isin = isin && _min[i] <= p[i] && _max[i] >= p[i];
      return isin;
   }
   bool in(Point3D p) {
      if( !_hasContent ) return false;
      bool isin = true;
      for( t_TAPINT i = 0; i < 3; i++ )
         isin = isin && _min[i] <= p[i] && _max[i] >= p[i];
      return isin;
   }
};


//for normalized display in cartesian space
class gRectangle { 
protected:
   Point2D tl;
   Point2D br;
   
public:
   t_TAPFLOAT x() {return tl[0];   }
   t_TAPFLOAT y() {return tl[1];   }
   t_TAPFLOAT w() {return br[0] - tl[0];     }
   t_TAPFLOAT h() {return tl[1] - br[1];     }
   
   void   setx(t_TAPFLOAT x) { tl[0] = x; }
   void   sety(t_TAPFLOAT y) { tl[1] = y; }
   void   setw(t_TAPFLOAT w) { br[0] = tl[0]+w; }
   void   seth(t_TAPFLOAT h) { br[1] = tl[1]-h; }
   
   t_TAPFLOAT left()              { return tl[0]; }
   t_TAPFLOAT right()             { return br[0]; }
   t_TAPFLOAT top()               { return tl[1]; }
   t_TAPFLOAT bottom()            { return br[1]; }

   void   setPos(Point2D p)   { moveto(p[0],p[1]); }
   Point2D pos()              { return tl; }
   Point2D center()           { return ( tl + br )* 0.5 ; }
   void   setl(t_TAPFLOAT l)      { tl[0] = l; }
   void   setr(t_TAPFLOAT r)      { br[0] = r; }
   void   sett(t_TAPFLOAT t)      { tl[1] = t; }
   void   setb(t_TAPFLOAT b)      { br[1] = b; }
   
   void moveto(t_TAPFLOAT x, t_TAPFLOAT y) { br += Point2D(x,y) - tl; tl = Point2D(x,y); }
   void resize(t_TAPFLOAT w, t_TAPFLOAT h) { br = tl + Point2D(w, -h); }
   
   bool   inside (Point2D pt) { return ( tl[0] < pt[0] && pt[0] < br[0] && br[1] < pt[1] && pt[1] < tl[1] ) ? true : false; }  
   
   void print() { tl.printnl(); br.printnl(); }
   gRectangle() : tl(Point2D(0,0)), br(Point2D(1,-1)) { } 
   gRectangle(t_TAPFLOAT x, t_TAPFLOAT y, t_TAPFLOAT w, t_TAPFLOAT h) : tl(Point2D(x,y)), br(Point2D(x+w,y-h)) {} 
   t_TAPFLOAT douter(Point2D p); 
};

class Circle { 
public:
   Point3D p;
   t_TAPFLOAT rad;
   Circle() { p = Point3D(0.0,0.0,0.0); rad = 1.0;}
   Circle(Point3D p1, Point3D p2, Point3D p3);
   bool inside(Point3D p);
};


/* A ray is a directed line segment which is defined by a pair of 3D
 * coordinates (or points). The first point specifies the starting point of the
 * ray and the second specifies its direction. It supports methods for
 * translating, finding the paramtetrized distance along a ray, and taking
 * the dot product. */

/* In terms of the position operators are equivalent to:
 *  r(s)=r.position(s)=r.p+r.q*s;
 *  r.dot(m)=(r.q-r.p).dot(m)
 * where "s" is floating point number and "m" is Point3D. (You don't need them
 * but they may help.) */



class Ray{
 public:
  Point3D p;
  Point3D d;

  Ray(){;}
  Ray(Point3D p1,Point3D d1){p=p1;d=d1;}

  void print(void){p.print();printf(":");d.print();}
  void printnl(void){print();printf("\n");}

  Ray translate(Point3D pt);

  Point3D operator() (Flt param);
  Point3D position(Flt param);
};

/* The Matrix class is a 4x4 matrix and supports: taking of determinant,
 * multiplying by another matrix on the right, transposing, inverting,
 * transforming a position, transforming a direction, transforming a normal,
 * and transforming a ray */

/* Its elements are indexed using parentheses and are indexed first by column
 * and the by row. Thus:
 *    Matrix m;
 *    t_TAPFLOAT  f;
 *    ...
 *    f=m(i,j);
 * Sets "f" equal to the value in the matrix stored in column "i" and row "j".
 * (Indexing starts at zero.) */

/* Note that since the matrix is actually a 4x4 matrix it is also used to
 * represent translations. For this reason it supports multiplication by the
 * three different kind of points:
 *  1) Multiplying a positional point performs the full transformation
 *  2) Multiplying a direction does not perform the translation, and
 *  3) Multiplying a normal returns a unit-vector which corresponds to the
 *     transformed normal. (i.e. If "n" is a vector normal to a plane then
 *     Matrix.multNormal(n) returns a unit-vector that is normal to the 
 *     transformed plane.) */

/* Indexed via Matrix(col,row)*/

#define RLUM    (0.3086)
#define GLUM    (0.6094)
#define BLUM    (0.0820)


class Matrix{
  Flt subDet(t_TAPINT col1,t_TAPINT row1,t_TAPINT col2,t_TAPINT row2);
  Flt subDet(t_TAPINT col,t_TAPINT row);
  Flt m[4][4];
 public:
  Flt& operator() (t_TAPINT col,t_TAPINT row);

  Flt det(void);

  void  fill3x3mat(t_TAPFLOAT n[3][3]);
  void  saturate(t_TAPFLOAT sat);
  void  huerotate(t_TAPFLOAT angle);
  void  luminance();
  void  cscale( t_TAPFLOAT r, t_TAPFLOAT g, t_TAPFLOAT b, t_TAPFLOAT a=1);

  void  rotate(t_TAPINT axis, t_TAPFLOAT rs, t_TAPFLOAT rc);
  void  zshear(t_TAPFLOAT dx, t_TAPFLOAT dy);

  void print(void);
  void printnl(void){print();printf("\n");}

  void readgl(t_TAPFLOAT *mt) { 
     for ( t_TAPINT col = 0; col < 4 ; col++) { 
        for( t_TAPINT row = 0 ; row < 4 ; row++ ) { 
            m[col][row] = mt[col*4 + row];
        }
     }
  }

  void fillgl(t_TAPFLOAT *mt, t_TAPINT dim ) { 
     for ( t_TAPINT col = 0; col < 4 ; col++) { 
        for ( t_TAPINT row = 0 ; row < 4 ; row++ ) { 
            mt[col*4 + row] = m[col][row];;
        }
     }
  }

  Matrix scale(t_TAPFLOAT f);
  Matrix mult(Matrix m);
  Matrix sum(Matrix b);
  Matrix operator* (Matrix m);

  Matrix operator* (t_TAPFLOAT d) {return scale(d);}
  Matrix operator+ (Matrix m) {return sum(m); } 

  Matrix transpose(void);

  Matrix invert(void);

  Point3D multPosition(Point3D position);
  Point3D multDirection(Point3D direction);
  Point3D multNormal(Point3D normal);

  Ray mult(Ray ray);
  Ray operator* (Ray ray);

  t_TAPINT isIdentity(void);
};
Matrix IdentityMatrix(void);
Matrix LuminanceMatrix(void);


#endif
