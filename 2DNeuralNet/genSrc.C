//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
vector<TCrown*> GenPgons(Float_t arm_p = 0.03) {
  vector<TCrown*> allPgons;

  Int_t n_pix[] = { 1, 5, 10, 15, 20, 25, 25, 25 };
  Float_t gap_r = 0.; //0.001;
  Float_t s_rad = 0.;

  for (Int_t j = 0; j < sizeof(n_pix) / sizeof(*n_pix); j++) { // loop on layers
    Float_t f_rad = s_rad + 2 * arm_p;
    Float_t gap_a = 0.; //TMath::RadToDeg() * arm_p / (4 * f_rad * TMath::Pi());
    if (j < 1) {
      gap_a = 0.;
      f_rad = s_rad + arm_p;
    }

    Float_t d_ang = 360. / n_pix[j];
    Float_t s_ang = 90.;
    if (!(j % 2))
      s_ang += d_ang / 2;

    for (Int_t i = 0; i < n_pix[j]; i++) { // loop on pixels
      Float_t f_ang = s_ang - d_ang;
      allPgons.push_back(new TCrown(0.5, 0.5, s_rad, f_rad - gap_r, s_ang - gap_a / 2, f_ang + gap_a / 2));
      s_ang = f_ang;
    }

    s_rad = f_rad;
  }

  for (auto p : allPgons) {
    p->SetFillStyle(1001);
    p->SetFillColor(kRed - 7);
    p->SetLineWidth(0);
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  return allPgons;
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// I/O info
TString pixMapData = "/home/ritabrata/Work/CrystalEye/Analysis/NeuralNet/Projection/pixPolToUpCrystalMap-V2R8.txt";
TString tFilePath = "/home/ritabrata/Work/CrystalEye/Analysis/Localization/V2R8/DataSamSet7/";
TString posInfoData = "/home/ritabrata/Work/CrystalEye/Prod/Localization/V2R8/srcpos-sam-set7.txt";
TString outBaseName = "src-set7";
const Int_t RES = 80; // resolution of the projected image
const Bool_t subtractBkg = 0;
const Bool_t verbose = 1;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void genSrc(const UShort_t nOcc = 1000, const ULong_t seed = 1) {
  // Get the projected geom
  Float_t arm_p = 0.033;
  auto allPgon = GenPgons(arm_p);

  // Get the pixel count rates for background
  auto pixData = Background();

  // Check pixel nos.
  const Int_t nPix = pixData->GetXaxis()->GetNbins();
  if (nPix != allPgon.size()) {
    cout << "No. of pixels do not match!!" << endl;
    return;
  }

  // Acd to up pixel map
  fstream acdMap(pixMapData.Data());
  Int_t iAcd, iUp;
  Int_t rmap[nPix];

  while (1) {
    acdMap >> iAcd >> iUp;
    if (acdMap.eof())
      break;
    rmap[iAcd] = iUp;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add src and bkg contribution and creat 3D histos
  // Get the src data files
  TString tFileName;
  vector < TString > dataFiles;
  for (const auto &entry : std::filesystem::directory_iterator(tFilePath.Data())) {
    tFileName = entry.path().string().c_str();
    if (tFileName.Contains(".root"))
      dataFiles.push_back(tFileName);
  }

  // Check position information file is right
  auto tThPh = new TTree("tThPh", "read theta phi");
  Int_t nf = tThPh->ReadFile(posInfoData.Data(), "n/C:t/F:p");
  char nam[100];
  tThPh->SetBranchAddress("n", nam);
  for (int nO = 0; nO < nf; nO++) {
    tThPh->GetEntry(nO);

    tFileName.Form("%s.root", nam);
    if (!gSystem->FindFile(tFilePath.Data(), tFileName)) {
      cout << "Position information file is not correct!...\n";
      exit(1);
    }
  }

  // Add src contribution
  const Int_t nP = (nOcc < nf) ? nOcc : nf;
  TH1F *hEdepPix[nP];
  TH2F *hPix[nP];
  TH1F *hSrcBkgPix[nP];
  std::string name, title;
  gRandom->SetSeed(seed);

  for (int nO = 0; nO < nP; nO++) {
    // Select random data file if less than existing files to be analyzed
    if (nP < nf)
      tFileName = dataFiles[nf * gRandom->Rndm()];
    else
      tFileName = dataFiles[nO];

    gSystem->FindFile(tFilePath.Data(), tFileName);

    auto tFile = new TFile(tFileName);
    hEdepPix[nO] = (TH1F*) tFile->Get("hNormEdepPix");
    hSrcBkgPix[nO] = (TH1F*) hEdepPix[nO]->Clone();

    // For background subtraction
    if (subtractBkg) {

      hSrcBkgPix[nO]->Reset();

      for (Int_t j = 1; j <= hSrcBkgPix[nO]->GetNbinsX(); j++) {
        Float_t cnt = hEdepPix[nO]->GetBinContent(j)
            + gRandom->Gaus(pixData->GetBinContent(j), pixData->GetBinError(j));
        cnt = cnt - gRandom->Gaus(pixData->GetBinContent(j), pixData->GetBinError(j));
        cnt = (cnt > 0) ? cnt : 0;
        hSrcBkgPix[nO]->SetBinContent(j, cnt);
        hSrcBkgPix[nO]->SetBinError(j, sqrt(pow(hEdepPix[nO]->GetBinError(j), 2) + pow(pixData->GetBinError(j), 2)));
      }
    }

    if (verbose)
      cout << "Generating src pixel map " << nO + 1 << "/" << nP << ". src file: " << tFileName << endl;

    name = "hPix" + std::to_string(nO);
    title = gSystem->BaseName(tFileName);
    title += ";X;Y;Normalized Counts/s";
    hPix[nO] = new TH2F(name.data(), title.data(), RES, 0, 1, RES, 0, 1);

    for (Int_t i = 1; i <= hPix[nO]->GetNbinsX(); i++)
      for (Int_t j = 1; j <= hPix[nO]->GetNbinsY(); j++) {
        Float_t px = hPix[nO]->GetXaxis()->GetBinCenter(i);
        Float_t py = hPix[nO]->GetYaxis()->GetBinCenter(j);

        Int_t ind = 0;
        for (auto p : allPgon) {
          if (p->IsInside(px, py)) {
            hPix[nO]->SetBinContent(i, j, hSrcBkgPix[nO]->GetBinContent(rmap[ind] + 1));
            continue;
          }
          ind++;
        }
      }

    hPix[nO]->Scale(1. / hPix[nO]->GetMaximum());

  }

  // Write data
  name = outBaseName.Data();
  name += "-seed" + std::to_string(seed) + ".root";
  TFile *ofile = new TFile(name.data(), "RECREATE");
  ofile->cd();
  for (auto h : hPix)
    h->Write();
  TNamed n("srcPosInfo", posInfoData.Data());
  n.Write();
  ofile->Close();

  // Draw
  if (verbose) {
    TCanvas *can = new TCanvas("can", "Polar projection", 800, 800);
    gStyle->SetOptStat(0);
    hPix[0]->Draw("colz");
  }

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
