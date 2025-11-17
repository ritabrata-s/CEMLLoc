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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// I/O info
TString tFilePath = "/home/ritabrata/Work/CrystalEye/Analysis/Localization/V2R8/DataSamSet7/";
TString posInfoData = "/home/ritabrata/Work/CrystalEye/Prod/Localization/V2R8/srcpos-sam-set7.txt";
TString outBaseName = "src-set7";
const Bool_t subtractBkg = 1;
const Bool_t verbose = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void genSrc(const UShort_t nOcc = 1000, const ULong_t seed = 1) {

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

  const Int_t nP = (nOcc < nf) ? nOcc : nf;

  TH1F *hEdepPix[nP];
  TH1F *hSrcBkgPix[nP];
  std::string title;
  gRandom->SetSeed(seed);
  const Int_t nB = 50;
  const Float_t rad = 175.;
  string name;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the pixel count rates for background
  TH1F *pixData = nullptr;
  if (subtractBkg)
    pixData = Background();

  // Loop on the src files
  for (int nO = 0; nO < nP; nO++) {
    // Select random data file if less than existing files to be analyzed
    if (nP < nf)
      tFileName = dataFiles[nf * gRandom->Rndm()];
    else
      tFileName = dataFiles[nO];

    // Get the src data file and the pixel histo
    gSystem->FindFile(tFilePath.Data(), tFileName);
    auto tFile = new TFile(tFileName);
    hEdepPix[nO] = (TH1F*) tFile->Get("hNormEdepPix");
    hSrcBkgPix[nO] = (TH1F*) hEdepPix[nO]->Clone();

    // For background subtraction
    if (subtractBkg) {

      hSrcBkgPix[nO]->Reset();

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Add the src and bkg data. Bkg data is also added with random Gaussian fluctuation.
      for (Int_t j = 1; j <= hSrcBkgPix[nO]->GetNbinsX(); j++) {
        Float_t cnt = hEdepPix[nO]->GetBinContent(j)
            + gRandom->Gaus(pixData->GetBinContent(j), pixData->GetBinError(j));
        // Subtract the bkg
        cnt = cnt - gRandom->Gaus(pixData->GetBinContent(j), pixData->GetBinError(j));
        cnt = (cnt > 0) ? cnt : 0;
        hSrcBkgPix[nO]->SetBinContent(j, cnt);
        // Recalculate the error
        hSrcBkgPix[nO]->SetBinError(j, sqrt(pow(hEdepPix[nO]->GetBinError(j), 2) + pow(pixData->GetBinError(j), 2)));
      }

    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Generate the 3D histos
    if (verbose)
      cout << "Generating src pixel map " << nO + 1 << "/" << nP << ". src file: " << tFileName << endl;

    name = "hPix" + std::to_string(nO);
    title = gSystem->BaseName(tFileName);
    title += ";Pixel Id.;Normalized Counts/s";
    hSrcBkgPix[nO]->SetName(name.data());
    hSrcBkgPix[nO]->SetTitle(title.data());

  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Draw
  if (verbose) {
    TCanvas *can = new TCanvas("can", "Pixel cnt", 800, 800);
    gStyle->SetOptStat(0);
    hSrcBkgPix[0]->Draw("colz");
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write data
  name = outBaseName.Data();
  name += "-seed" + std::to_string(seed) + ".root";
  TFile *ofile = new TFile(name.data(), "RECREATE");
  ofile->cd();
  for (auto h : hSrcBkgPix)
    h->Write();
  TNamed n("srcPosInfo", posInfoData.Data());
  n.Write();
  ofile->Close();

}
