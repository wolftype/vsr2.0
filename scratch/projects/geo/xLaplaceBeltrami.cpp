/*
 * =====================================================================================
 *
 *       Filename:  xLaplaceBeltrami.cpp
 *
 *    Description:  test of cotan formula with recip frame
 *
 *        Version:  1.0
 *        Created:  08/18/2014 13:41:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Pablo Colapinto (), gmail -> wolftype
 *
 * =====================================================================================
 */

#include "vsr/vsr_app.h"

using namespace vsr;
using namespace vsr::cga;
using namespace gfx;

struct MyApp : App
{

  Pnt mouse;
  Lin ray;

  float time = 0;
  float amt, amtB;

  bool bReset, bUseRecip;

  Frame frame;

  void setup ()
  {
    gui (amt, "amt", -100, 100);
    gui (amtB, "amtB", -100, 100);
    gui (bReset, "reset") (bUseRecip, "bUseRecip");

    objectController.attach (&frame);
  }

  virtual void onDraw ()
  {

    mouse = calcMouse3D ();

    //Control frame
    Draw (frame.ty (), 0, 1, 0);

    //Create Vertices based on a Boost relative to a tangent
    //(Cause it's easy)
    int numverts = 7;
    vector<Pnt> pnt (numverts);
    for (int i = 0; i < numverts; ++i)
      {
        //spin Vec::x about xz plane and put a frame there
        int tnum = numverts - 1;
        Pnt pa = i == tnum
                   ? PAO
                   : Vec (1, 0, 0).rot (Biv::xz * PI * amtB * i / tnum).null ();
        Frame tf (pa);

        //effect parameterized by inverse distance
        auto dist = 1.0 / Round::dist (tf.pos (), frame.pos ());
        auto bst = Gen::bst (frame.ty () * amt * dist);
        Par par = tf.ty ().spin (bst);
        Pnt np = Round::loc (par);
        pnt[i] = np;

        Draw (pnt[i]);
        Draw (par);
      }

    //center
    Pnt c = pnt[numverts - 1];

    Vec sum, gsum, dsum, vsum;
    Biv asum, bsum;
    float norm, dnorm;
    norm = 0;
    dnorm = 0;

    vector<Vec> edge (6);
    vector<Biv> tang (6);
    vector<Vec> grad (6);
    vector<Vec> grad2 (6);
    vector<Vec> grad3 (6);
    vector<Vec> cen (6);
    float wt = 0;
    float vwt = 0;
    float gwt = 0;
    for (int i = 0; i < numverts - 1; ++i)
      {
        int n = (i < numverts - 2) ? i + 1 : 0;
        int p = (i > 0) ? i - 1 : numverts - 2;

        //centers of adjacent triangles
        cen[i] = Round::loc (c ^ pnt[i] ^ pnt[n]);
        cen[p] = Round::loc (c ^ pnt[p] ^ pnt[i]);

        //valences
        Vec cur = Vec (pnt[i] - c);
        Vec next = Vec (pnt[n] - c);
        Vec prev = Vec (pnt[p] - c);

        //far edges
        Vec e = Vec (pnt[n] - pnt[i]);
        Vec e2 = Vec (pnt[i] - pnt[p]);

        //tangent spaces
        tang[i] = (cur ^ next);  //not weighted
        Biv tmp = prev ^ cur;

        // area
        float awt = tang[i].rnorm () / 2.0;
        wt += awt;

        //edge gradients
        Vec ea = -e <= (!tang[i]);
        Vec eb = -e2 <= (!tmp);

        // DrawAt(ea, Vec(pnt[i]) + Vec(pnt[n]-pnt[i])/2,0,1,1);
        // DrawAt(eb, Vec(pnt[p]) + Vec(pnt[i]-pnt[p])/2,0,1,1);

        // asum += area[i];

        //orthonormal basis of each simplex
        //recip = (-1)^i-1(b ..^)I^-1
        grad[i] = next <= (!tang[i]);
        grad2[i] = -prev <= (!tmp);

        float gcnorm =
          (((grad[i] * amt + grad2[i] * amt).norm ()) / cur.norm ());
        //or
        Vec eia = grad[i] * cur.norm ();
        Vec eib = grad2[i] * cur.norm ();
        Vec ei = eia + eib;

        //auto da = eia * (next<=cur)[0];
        //auto db = eib * (prev<=cur)[0];
        // gcnorm = ( (((grad[i]+ea)-(grad2[i]+eb) ) * amt ).norm() ) / cur.norm();
        // cout << "gc norm: " << i << " " << gcnorm << endl;

        //grad3[i] = -e<=(!area[i]);
        DrawAt (grad[i], Vec (c) + cur / 2, 1, 1, 0);
        DrawAt (grad2[i], Vec (c) + cur / 2, 0, 1, 0);


        // gcnorm = ei.norm();
        //ext_der.print();

        // DrawAt(Biv(ext_der), Vec(c)+cur/2,1,0,0);
        /* cout << "PROJ" << endl; */
        /* auto pja = (grad[i] <= dlp);//.print(); */
        /* auto pjb = (grad2[i]<=dlp);//.print(); */
        /* pja.print(); */
        //  bsum += (grad[i]^cur) + (grad2[i]^cur);

        // DrawAt( grad3[i], Vec(pnt[i]) + e/2,0,0,1);

        //  cout << "metric: " << (grad[i]<=cur)[0] << " " << (grad2[i]<=cur)[0]<< endl;
        edge[i] = cur;
        gwt += (grad3[i] <= grad3[i])[0];

        // DrawAt( grad[i], Vec(pnt[i]) + e/2.0 );
        //    auto cn = cur.norm();
        //        Vec va = Op::pj( grad[i], prev );//tmp.duale() );
        //        Vec vb = Op::pj( grad2[i], next );//area[i].duale() );
        //        Vec va = Op::pj( grad[i], e2 );//tmp.duale() );
        //        Vec vb = Op::pj( grad2[i], e );//area[i].duale() );
        Vec tv = ea / cur.norm ();
        //Op::pj(ei,cur)/cur.norm();//da + db;//cur*gcnorm;//(va+vb);//(grad[i]^grad2[i]).duale();
        DrawAt (tv, pnt[i], 0, 0, 1);
        gsum += bUseRecip ? tv : (cur * gcnorm);
        //( grad[i] + grad2[i] )/((area[i]/2).rnorm());


        auto dotA = Vec (pnt[i] - pnt[n]).unit () <= Vec (c - pnt[n]).unit ();
        auto dotB = Vec (pnt[i] - pnt[p]).unit () <= Vec (c - pnt[p]).unit ();
        auto ca = dotA[0];
        auto cb = dotB[0];
        auto angA = acos (ca);
        auto angB = acos (cb);
        auto sa = sin (angA);
        auto sb = sin (angB);

        auto cota = ca / sa;
        auto cotb = cb / sb;

        auto ct = (cota + cotb) * .5;
        norm += ct;

        // cout << i << " " << ct << endl;

        sum += Vec (pnt[i] - c) * ct;

        gfx::Glyph::Line (pnt[i], pnt[n]);
        gfx::Glyph::Line (c, pnt[i]);

        /* } */

        /* for (int i=0;i<6;++i){ */
        /* int n = (i<5) ? i+1 : 0; */
        /* int p = (i>0) ? i-1 : 5; */
        Vec dual = cen[i] - cen[p];
        //Vec e = Vec(pnt[i]-c);
        auto decnorm = dual.norm () / cur.norm ();

        // cout << "dec norm: " << i << " " << decnorm << endl;
        dsum += cur * (decnorm);
        dnorm += decnorm;
        // cout << i << " " << decnorm << endl;
      }

    float awt = (asum / 2).rnorm ();

    DrawAt (-dsum / (3.0), c, 1, 0, 1);
    DrawAt (-sum, c, 0, 1, 1);

    //  DrawAt( bsum , c, 1,1,0);
    DrawAt (-gsum, c, 1, 0, 0);

    cout << "NORMS: " << dnorm << " " << norm << " " << gwt << endl;
    cout << "cot: " << sum.wt () << endl;
    cout << "awt: " << awt << endl;
    cout << "grad: " << gsum.wt () << " " << gsum.wt () / wt << endl;
    cout << "dec: " << dsum.wt () << " " << dsum.wt () / (wt / 3.0) << endl;
  }
};



int main ()
{

  MyApp app;
  app.start ();

  return 0;
}
