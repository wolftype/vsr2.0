/*
 * =====================================================================================
 *
 *       Filename:  xReciprocal.cpp
 *
 *    Description:  Finding normals, it seems
 *
 *        Version:  1.0
 *        Created:  11/22/2013 13:33:48
 *       Revision:  none
 *       Compiler:  gcc4.7 or clang3.2 or greater (C++11)
 *
 *         Author:  Pablo Colapinto (), wolftype (gmail)
 *   Organization:  pretty awesome
 *
 * =====================================================================================
 */


#include "vsr/vsr_app.h"

using namespace vsr;
using namespace vsr::cga;
using namespace gfx;

#define INF Inf (1)

Dlp rframe (const Pnt &pa, const Pnt &pb)
{
  auto line = pa ^ pb ^ INF;
  auto norm = line.rnorm ();
  auto sqnorm = norm * norm;  //this is just the wt() (self dot product)

  auto rpa = (pb ^ INF) <= line;
  return rpa / -sqnorm;
}

vector<Dlp> rframe (const Pnt &pa, const Pnt &pb, const Pnt &pc)
{
  vector<Dlp> recip;
  //1. Make a "tangent space" (here the subspace pseudoscalar is a direct OPNS plane)
  auto plane = pa ^ pb ^ pc ^ INF;
  auto norm = plane.rnorm ();
  auto sqnorm = norm * norm;

  //2. A reciprocal frame is built from contraction of the the wedge of others with the plane pseudoscalar . . .
  auto rpa = ((pb ^ pc ^ INF) <= plane);
  auto rpb = ((pc ^ pa ^ INF) <= plane);
  auto rpc = ((pa ^ pb ^ INF) <= plane);

  //AND Normalized?
  rpa /= (-sqnorm);
  rpb /= (-sqnorm);
  rpc /= (-sqnorm);

  recip.push_back (rpa);
  recip.push_back (rpb);
  recip.push_back (rpc);

  return recip;
}

struct MyApp : App
{

  Pnt mouse;
  Lin ray;

  float time;
  float xpos, ypos, amtA, amtB, amtC, vNum;
  bool bDrawRecip, bNormalize, bAreaNormal, bAreaWeight;

  void setup ()
  {
    gui (xpos, "xpos", -100, 100);
    gui (ypos, "ypos", -100, 100);
    gui (amtA, "amtA", -1, 1);
    gui (amtC, "amtC", -1, 1);
    gui (vNum, "num", 1, 100);
    gui (bDrawRecip, "drawRecip") (bNormalize,
                                   "normalize") (bAreaNormal,
                                                 "areaNormal") (bAreaWeight,
                                                                "areaWeight");

    bAreaNormal = true;
    bAreaWeight = false;
  }

  virtual void onDraw ()
  {

    mouse = calcMouse3D ();

    Pnt vertex = Round::null (xpos, ypos, 0);

    //Nbrs
    vector<Pnt> nbr;
    int num = vNum;
    for (int i = 0; i < num; ++i)
      {
        float t = (float) i / num;
        nbr.push_back (Round::null (Vec::x.rot (Biv::xz * (PIOVERTWO * t))));
        Draw (nbr[i], 1, 0, 0);
      }

    //Directed content a_k
    vector<Biv> dm;
    Biv bdm;
    for (int i = 0; i < num; ++i)
      {
        int next = i < (num - 1) ? i + 1 : 0;
        Vec va = nbr[i] - vertex;
        Vec vb = nbr[next] - vertex;
        dm.push_back (va ^ vb * .5);
        bdm += va ^ vb * .5;
      }

    //Reciprocals
    vector<Dlp> rp;
    Dlp brp;
    for (int i = 0; i < num; i++)
      {
        int next = i < (num - 1) ? i + 1 : 0;
        //rframe edge
        // auto trp = rframe( nbr[i], vertex ) + Inf(amtC) ;
        //rframe face
        auto trp = rframe (vertex, nbr[i], nbr[next])[0] + Inf (amtC);
        //select
        auto frp = trp;  //bAreaNormal ? trp2 : trp;
        rp.push_back (frp);
        brp += frp / num;
        if (bDrawRecip)
          Draw (frp, 0, 1, 1, .4);
      }

    //Moments
    Flp flp;
    for (int i = 0; i < num; i++)
      {
        //this is a flat point support vector
        auto moment = dm[i] <= rp[i].dual ();
        flp += moment;
        DrawAt (moment.copy<Vec> (), nbr[i], 0, 1, 0);
        DrawAt (dm[i].duale (), nbr[i], 1, 1, 0);
      }

    DrawAt (flp.copy<Vec> (), vertex, 0, 1, 0);

    DrawAt (Vec (brp), vertex, 1, 0, 1);

    //collective action
    //      auto sumflp = bdm <= brp.dual();
    //      DrawAt( sumflp.copy<Vec>(), vertex, 1,0,1);

    //Draw
    for (int i = 0; i < num; i++)
      {
        int next = i < (num - 1) ? i + 1 : 0;
        glColor3f (1, 0, 0);
        gfx::Glyph::Line (vertex, nbr[i]);
        gfx::Glyph::Line (nbr[i], nbr[next]);
      }


    /* Dlp sum; */
    /* for (int i=0; i<num; ++i){ */
    /*   sum += ( rp[i] / ( bAreaWeight ? rp[i].norm() : 1 ) ) + Inf(amtA); */
    /* } */

    /* auto norm =  sum.norm(); */
    /* if (bNormalize) sum /= norm > FPERROR ? (norm * norm) : 1; */

    //sum.runit();

    // Draw( sum,0,1,0);
    // DrawAt( Vec(sum), vertex,0,1,1);


    //intersect tangent space with ray
    /* auto intersect = ( ray.dual() ^ plane.dual() ).dual().unit().null(); */

    /* auto sa = ( rpa <= intersect )[0]; */
    /* auto sb = ( rpb <= intersect )[0]; */
    /* auto sc = ( rpc <= intersect )[0]; */

    /* if ( sa >= 0 && sa <= 1 && sb >=0 && sb <=1 && sc>=0 && sc<=1){ */
    /*   Draw( ray,1,0,0); */
    /*   Draw ( ray.reflect( plane ),0,1,0 ); */
    /* } */
  }
};



int main ()
{

  MyApp app;
  app.start ();

  return 0;
}
