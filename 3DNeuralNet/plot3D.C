//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TGeoManager* geom() {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Detector geometry
  // GeomManagere
  TGeoManager *cegeom = new TGeoManager("cegeom", "CE detector");

  // Materials
  TGeoMaterial *vacuum = new TGeoMaterial("vacuum", 0, 0, 0);
  TGeoMaterial *Fe = new TGeoMaterial("Fe", 55.845, 26, 7.87);

  TGeoMedium *Air = new TGeoMedium("Vacuum", 0, vacuum);
  TGeoMedium *Iron = new TGeoMedium("Iron", 1, Fe);

  // Top volume
  TGeoVolume *top = cegeom->MakeBox("top", Air, 500, 500, 500);
  cegeom->SetTopVolume(top);
  cegeom->SetTopVisible(false);

  // Pixel parameters
  Float_t detRad = 167.; // radius of the detector in mm
  Float_t pixGap = 0; //2; // pixel gap in mm
  Float_t pixTh = 4.; // pixel thickness in mm

  // Convert chord length to arc length
  Float_t apixGap = 2 * detRad * asin(pixGap / (2 * detRad));

  Float_t pixArm = ((TMath::Pi() * detRad / 2) - 7 * apixGap)
      / (3 + 8 * cos(30. * TMath::DegToRad()) + (1. / tan(36. * TMath::DegToRad()))
          + (0.5 / sin(36. * TMath::DegToRad())));
//  Float_t pixArm = ((TMath::Pi() * detRad / 2) - 8 * pixGap)
//      / (3 + 8 * cos(30. * TMath::DegToRad()) + (1. / tan(36. * TMath::DegToRad())));

  Float_t penRad = pixArm / (2 * tan(36. * TMath::DegToRad()));
  Float_t hexRad = pixArm * cos(30. * TMath::DegToRad());
  cout << pixArm << "\t" << penRad << "\t" << hexRad << endl;

  // Pentagonal and hexagonal volumes and shapes
  TGeoVolume *vpen = cegeom->MakePgon("vpen", Iron, 0, 360, 5, 2);
  vpen->SetLineColor(12);
  vpen->SetFillColor(12);
  TGeoPgon *spen = (TGeoPgon*) (vpen->GetShape());
  spen->DefineSection(0, 0, 0, penRad);
  spen->DefineSection(1, pixTh, 0, penRad);

  TGeoVolume *vhex = cegeom->MakePgon("vhex", Iron, 0, 360, 6, 2);
  vhex->SetLineColor(18);
  vhex->SetFillColor(18);
  TGeoPgon *shex = (TGeoPgon*) (vhex->GetShape());
  shex->DefineSection(0, 0, 0, hexRad);
  shex->DefineSection(1, pixTh, 0, hexRad);

  // Theta position of different volume layers (in deg)
  // First convert chord lengths to arc lengths
  Float_t apenRad = 2 * detRad * asin(penRad / (2 * detRad));
  Float_t ahexRad = 2 * detRad * asin(hexRad / (2 * detRad));
  Float_t apixArm = 2 * detRad * asin(pixArm / (2 * detRad));
  Float_t apenVer = 2 * detRad * asin((penRad / cos(36. * TMath::DegToRad())) / (2 * detRad));
  Float_t apixProj = 2 * detRad * asin(pixArm * sin(15. * TMath::DegToRad()) / (2 * detRad));

  Float_t posThP1 = TMath::RadToDeg() * (2 * apenRad + 8 * ahexRad + 6 * apixGap) / detRad; // pentagons away from the center
  Float_t posThH1 = TMath::RadToDeg() * (apenRad + ahexRad + apixGap) / detRad; // first hexagons surrounding the central pentagon
  Float_t posThH21 = TMath::RadToDeg() * (apenRad + 3 * ahexRad + 2 * apixGap) / detRad; // second hexagons at arms surrounding the central pentagon
  Float_t posThH22 = TMath::RadToDeg() * (apenVer + 2 * apixArm + apixGap) / detRad; // second hexagons at corners surrounding the central pentagon
  Float_t posThH3 = TMath::RadToDeg() * (apixArm + 0.5 * apixGap) / detRad; // triple hexagons w.r.t. their common center
  Float_t posThHU1 = TMath::RadToDeg() * (apenVer + 4 * apixArm + 2 * apixGap) / detRad; // first layer of triple hex units arround the center
  Float_t posThHU2 = TMath::RadToDeg() * (2 * apenRad + 8 * ahexRad + 6.5 * apixGap + apenVer + apixArm + apixProj)
      / detRad; // second layer of triple hex units arround the center
//  Float_t posThP1 = TMath::RadToDeg() * (2 * penRad + 8 * hexRad + 6 * pixGap) / detRad; // pentagons away from the center
//  Float_t posThH1 = TMath::RadToDeg() * (penRad + hexRad + pixGap) / detRad; // first hexagons surrounding the central pentagon
//  Float_t posThH21 = TMath::RadToDeg() * (penRad + 3 * hexRad + 2 * pixGap) / detRad; // second hexagons at arms surrounding the central pentagon
//  Float_t posThH22 = TMath::RadToDeg() * (penRad / cos(36. * TMath::DegToRad()) + 2 * pixArm + pixGap) / detRad; // second hexagons at corners surrounding the central pentagon
//  Float_t posThH3 = TMath::RadToDeg() * (pixArm + 0.5 * pixGap) / detRad; // triple hexagons w.r.t. their common center
//  Float_t posThHU1 = TMath::RadToDeg() * (penRad / cos(36. * TMath::DegToRad()) + 4 * pixArm + 2 * pixGap) / detRad; // first layer of triple hex units arround the center
//  Float_t posThHU2 = TMath::RadToDeg()
//      * (2 * penRad + 8 * hexRad + 6.5 * pixGap + (penRad / cos(36. * TMath::DegToRad())) + pixArm + pixArm*sin(15. * TMath::DegToRad())
//          ) / detRad; // second layer of triple hex units arround the center
//  Float_t posThHU2 = TMath::RadToDeg() * (penRad / cos(36. * TMath::DegToRad()) + 10 * pixArm + 1 * pixGap) / detRad; // second layer of triple hex units arround the center

  Float_t posPh, posTh, posPhRad, posThRad, posX, posY, posZ;
  Int_t ind(0);
  TString name;
  TGeoVolume *vol[126];

  // The central pentagon
  posTh = 0;
  posPh = 90;
  posThRad = posTh * TMath::DegToRad();
  posPhRad = posPh * TMath::DegToRad();
  posX = detRad * sin(posThRad) * cos(posPhRad);
  posY = detRad * sin(posThRad) * sin(posPhRad);
  posZ = detRad * cos(posThRad);

  name.Form("Pix%d", ind++);
  top->AddNode(new TGeoVolume(name, spen, Iron), 1,
      new TGeoCombiTrans(posX, posY, posZ, new TGeoRotation("rot", posPh - 90., -posTh, 90)));
//  cout << name << "\t" << posTh << "\t" << posPh << endl;

  // Hexagons arround the central pentagon
  Float_t posThH[3] = { posThH1, posThH21, posThH22 }; // three layers
  Float_t posPhH[3] = { 54., 54., 54. + 36. };
  Float_t psi[3] = { 0, 0, 90. };
  TVector3 vecH[15], vecHU[2][3];

  for (Int_t j = 0; j < 3; j++)
    for (Int_t k = 0; k < 5; k++) {
      posTh = posThH[j];
      posPh = posPhH[j] + k * 72;
      posThRad = posTh * TMath::DegToRad();
      posPhRad = posPh * TMath::DegToRad();
      posX = detRad * sin(posThRad) * cos(posPhRad);
      posY = detRad * sin(posThRad) * sin(posPhRad);
      posZ = detRad * cos(posThRad);
      vecH[j * 5 + k] = TVector3(posX, posY, posZ);
      vecH[j * 5 + k].RotateZ((-54. + 36) * TMath::DegToRad());
      name.Form("Pix%d", ind++);
      top->AddNode(new TGeoVolume(name, shex, Iron), 1,
          new TGeoCombiTrans(posX, posY, posZ, new TGeoRotation("rot", posPh - 90, -posTh, psi[j])));
//      cout << name << "\t" << posTh << "\t" << posPh << endl;
    }

  // Triple hexagon units
  for (Int_t j = 0; j < 2; j++)
    for (Int_t k = 0; k < 3; k++) {
      posTh = posThH3;
      posPh = j * 60. + k * 120;
      posThRad = posTh * TMath::DegToRad();
      posPhRad = posPh * TMath::DegToRad();
      posX = detRad * sin(posThRad) * cos(posPhRad);
      posY = detRad * sin(posThRad) * sin(posPhRad);
      posZ = detRad * cos(posThRad);
      vecHU[j][k] = TVector3(posX, posY, posZ);
    }

  // Five periferal pentagons and its surrounding hexagons
  // Rotation of the hexagons w.r.t. local z-axis
  Float_t rotH[3][5] = { { -27., -18., 0., 18., 27. }, { -29., -26., 0., 26., 29. }, { -27., -16., 16., 27., 29. } }; // hexagons around each penta
  Float_t rotHU[2][3] = { { -30., 20., 40. }, { -31., 30., 31. } }; // for two layers of the triple hexa units
  TVector3 mvec[15], mvec2[3];
  TVector3 dir;
  Float_t posThHU[2] = { posThHU1, posThHU2 };

  // Loop over the 5 penta
  for (Int_t i = 0; i < 5; i++) {
    posTh = posThP1;
    posPh = 54 + i * 72.;
    posThRad = posTh * TMath::DegToRad();
    posPhRad = posPh * TMath::DegToRad();
    posX = detRad * sin(posThRad) * cos(posPhRad);
    posY = detRad * sin(posThRad) * sin(posPhRad);
    posZ = detRad * cos(posThRad);

    name.Form("Pix%d", ind++);
    top->AddNode(new TGeoVolume(name, spen, Iron), 1,
        new TGeoCombiTrans(posX, posY, posZ, new TGeoRotation("rot", posPh - 90., -posTh, 90)));
//    cout << name << "\t" << posTh << "\t" << posPh << endl;

    // Loop on first 5 hexagon positions
    Float_t posThP1Rad = posThRad;
    Float_t posPhP1Rad = posPhRad;
    std::copy(std::begin(vecH), std::end(vecH), std::begin(mvec));

    // Loop over the hexa layers
    for (Int_t j = 0; j < 3; j++)
      for (Int_t k = 0; k < 5; k++) {
        dir.SetMagThetaPhi(1, posThP1Rad, posPhP1Rad);
        mvec[j * 5 + k].RotateUz(dir.Unit());

        posTh = TMath::RadToDeg() * mvec[j * 5 + k].Theta();
        posPh = TMath::RadToDeg() * mvec[j * 5 + k].Phi();
        posX = mvec[j * 5 + k].X();
        posY = mvec[j * 5 + k].Y();
        posZ = mvec[j * 5 + k].Z();

        name.Form("Pix%d", ind++);
        top->AddNode(new TGeoVolume(name, shex, Iron), 1,
            new TGeoCombiTrans(posX, posY, posZ, new TGeoRotation("rot", posPh - 90, -posTh, rotH[j][k])));
//        cout << name << "\t" << posTh << "\t" << posPh << endl;
      }

    // Layers of triple hexa units
    for (Int_t j = 0; j < 2; j++) {
      posTh = posThHU[j];
      posPh = 18 + i * 72.;
      posThRad = posTh * TMath::DegToRad();
      posPhRad = posPh * TMath::DegToRad();
      posX = detRad * sin(posThRad) * cos(posPhRad);
      posY = detRad * sin(posThRad) * sin(posPhRad);
      posZ = detRad * cos(posThRad);

      // Loop on first 5 hexagon positions
      Float_t posThT1Rad = posThRad;
      Float_t posPhT1Rad = posPhRad;
      std::copy(std::begin(vecHU[j]), std::end(vecHU[j]), std::begin(mvec2));

      for (Int_t k = 0; k < 3; k++) {
        dir.SetMagThetaPhi(1, posThT1Rad, posPhT1Rad);
        mvec2[k].RotateUz(dir.Unit());

        posTh = TMath::RadToDeg() * mvec2[k].Theta();
        posPh = TMath::RadToDeg() * mvec2[k].Phi();
        posX = mvec2[k].X();
        posY = mvec2[k].Y();
        posZ = mvec2[k].Z();

        name.Form("Pix%d", ind++);
        top->AddNode(new TGeoVolume(name, shex, Iron), 1,
            new TGeoCombiTrans(posX, posY, posZ, new TGeoRotation("rot", posPh - 90, -posTh, rotHU[j][k])));
//        cout << name << "\t" << posTh << "\t" << posPh << endl;
      }
    }

  }

  cegeom->CloseGeometry();

  return cegeom;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TH1F* Background() {
  // Background data files
  TString fPath = "/home/ritabrata/Work/CrystalEye/Analysis/Config2/Bkg/Study8/GAGGLYSO/";
  TString fNames[] = { "Gamma/Downward/gamma-uhs-v2r8-TT.root", // 0
      "Gamma/Upward/gamma-all-v2r8-TT.root", // 1
      "Neutron/neutron-all-v2r8-TT.root", // 2
      "Electron/electron-all-v2r8-TT.root", // 3
      "Positron/positron-all-v2r8-TT.root", // 4
      "Proton/proton-all-v2r8-TT.root", // 5
      "Proton/proton-uhs-4-100G-v2r8-TT.root" // 6
      //"Intrinsic/int-TT.root" // 7
//      "../../../../Localization/V2R8/Data5deg/src-42-0.root" //
      };

  const unsigned NTYPE = sizeof(fNames) / sizeof(fNames[0]);
  TFile *rfile[NTYPE];

  unsigned ind = 0;
  for (auto fname : fNames) {
    rfile[ind++] = new TFile(fPath + fname);
  }

  for (auto rf : rfile)
    if (!rf) {
      cout << "Input data file " << rf->GetName() << " not found!\n";
      return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get histogram of weighted counts of the edep pixels
  TH1F *hEdepPix[NTYPE];
  Int_t nt(0); // type of the bkg by number 0-5
  for (auto rf : rfile) {
    hEdepPix[nt++] = (TH1F*) rf->Get("hNormEdepPix");
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Total edep in pixels
  auto hEdepPixA = (TH1F*) hEdepPix[0]->Clone();
  for (Int_t i = 1; i < NTYPE; i++)
    hEdepPixA->Add(hEdepPix[i]);

  return hEdepPixA;
}

void plot3D() {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Colour definition
  const Int_t NCOL = 255;
  static Int_t colors[NCOL];

  Double_t Red[5] = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t Green[5] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t Blue[5] = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  Double_t Length[5] = { 0.00, 0.34, 0.61, 0.84, 1.00 };

  Int_t FI = TColor::CreateGradientColorTable(5, Length, Red, Green, Blue, NCOL);
  for (int i = 0; i < NCOL; i++)
    colors[i] = FI + i;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the pixel count rates for background
  auto pixData = Background();

  Float_t minV = pixData->GetMinimum();
  Float_t maxV = pixData->GetMaximum();
//  cout << minV << "\t" << maxV << endl;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 3d pix to up pixel map
  fstream pixMap("/home/ritabrata/Work/CrystalEye/Analysis/NeuralNet/3Dplot/pix3dToUpCrystalMap-V2R8.txt");
  Int_t iPix, iUp;
  Int_t map[126];

  while (1) {
    pixMap >> iPix >> iUp;
    if (pixMap.eof())
      break;
    map[iPix] = iUp;
//		map[iUp] = iPix;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  TGeoManager *cegeom = geom();
//  cegeom->GetVolume("Pix109")->SetLineColor(2);

  char name[100];

  for (Int_t i = 0; i < 126; i++) {
    sprintf(name, "Pix%d", i);
    // For each particle type (skip for all)
    Float_t cnt = pixData->GetBinContent(map[i] + 1);
    Int_t col = (Int_t)((cnt - minV) * (NCOL - 1) / (maxV - minV));
    TGeoVolume *vol = cegeom->GetVolume(name);
    vol->SetLineColor(colors[col]);
  }

  cegeom->GetTopVolume()->Draw("ogl");
//  TGLViewer::ECameraType camera = TGLViewer::kCameraOrthoXOY;
//  TGLViewer *v = (TGLViewer*) gPad->GetViewer3D();
//  v->SetCurrentCamera(camera);

//  return;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Histogram
  const Int_t nB = 100;
  const Float_t rad = 170;
  TH3F *hEdep = new TH3F("hEdep", ";X;Y;Z", nB, -rad, rad, nB, -rad, rad, nB / 2, 0, rad);

  for (int i = 1; i <= hEdep->GetNbinsX(); i++)
    for (int j = 1; j <= hEdep->GetNbinsY(); j++)
      for (int k = 1; k <= hEdep->GetNbinsZ(); k++) {
        Float_t xx = hEdep->GetXaxis()->GetBinCenter(i);
        Float_t yy = hEdep->GetYaxis()->GetBinCenter(j);
        Float_t zz = hEdep->GetZaxis()->GetBinCenter(k);
//        const Double_t pt[3] = { xx, yy, zz };
        TString node = cegeom->FindNode(xx, yy, zz)->GetVolume()->GetName();
        //					cout << xx << "\t" << yy << "\t" << zz << "\t" << node << endl;
        if (node.Contains("Pix")) {
          Float_t cnt = pixData->GetBinContent(map[node.Remove(0, 3).Atoi()] + 1);
          //					cout << cnt << "\t" << node << endl;
          hEdep->SetBinContent(i, j, k, cnt);
        }
      }

  // Draw
  TCanvas *can = new TCanvas("can", "Polar projection", 800, 800);
  gStyle->SetOptStat(0);
  hEdep->Draw("colz");

  // Write data
//  name = "bkg-seed" + std::to_string(seed) + ".root";
  sprintf(name,"bkg-hist3d.root");
  TFile *ofile = new TFile(name, "RECREATE");
  ofile->cd();
//  for (auto h : hPix)
  hEdep->Write();
  ofile->Close();

}
