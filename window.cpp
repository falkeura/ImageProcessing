#include <wx/wx.h>
#include "window.h"
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std; 

static const wxChar *FILETYPES = _T("All files (*.*)|*.*");

IMPLEMENT_APP(BasicApplication)

bool BasicApplication::OnInit()
{
  wxInitAllImageHandlers();
  MyFrame *frame = new MyFrame(_("Basic Frame"), 50, 50, 512, 512);
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;	
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width, int height): wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)){

  fileMenu = new wxMenu;
  fileMenu->Append(LOAD_FILE_ID, _T("&Open file"));  
  fileMenu->AppendSeparator();

//###########################################################//
//----------------------START MY MENU -----------------------//
//###########################################################// 

  fileMenu->Append(UNDO_ID, _T("&Undo"));
  fileMenu->Append(RESET_ID, _T("&Reset Image"));

  fileMenu->AppendSeparator();
  
  fileMenu->Append(INVERT_IMAGE_ID, _T("&Invert image"));
  fileMenu->Append(SHIFT_ID, _T("&Shift image")); 

  fileMenu->AppendSeparator();

  fileMenu->Append(BOXAVERAGE_ID, _T("&Convolution: Smooth Box Average image")); 
  fileMenu->Append(WEIGHTEDAVERAGE_ID, _T("&Convolution: Smooth Weighted Average image")); 
  fileMenu->Append(SOBELEDGE_ID, _T("&Sobel Edge detection")); 
  fileMenu->Append(ROBERTEDGE_ID, _T("&Robert Edge detection")); 

  fileMenu->AppendSeparator();

  fileMenu->Append(NOISE_ID, _T("&Add Salt and Pepper Noise"));  
  fileMenu->Append(MAX_ID, _T("&Order-Statistics: Max filter"));   
  fileMenu->Append(MIN_ID, _T("&Order-Statistics: Min filter"));    

  fileMenu->AppendSeparator();

  fileMenu->Append(NEGATIVE_ID, _T("&Negative Linear Transform"));
  fileMenu->Append(LOG_ID, _T("&Logarithmic Function"));
  fileMenu->Append(POWER_ID, _T("&Power Law Function"));

  fileMenu->AppendSeparator();

  fileMenu->Append(NORM_ID, _T("&Find and Normalise Histogram")); 
  fileMenu->Append(EQUALISE_ID, _T("&Equalise Histogram")); 

  fileMenu->AppendSeparator();

  fileMenu->Append(MEAN_ID, _T("&Find Mean and Standard Deviation")); 
  fileMenu->Append(THRESH_ID, _T("&Simple Thresholding"));

  fileMenu->AppendSeparator();

  fileMenu->Append(SETROI_ID, _T("&Set ROI"));
  fileMenu->Append(UNSETROI_ID, _T("&Unset ROI"));

//###########################################################//
//----------------------END MY MENU -------------------------//
//###########################################################// 

  fileMenu->AppendSeparator();
  fileMenu->Append(SAVE_IMAGE_ID, _T("&Save image"));
  fileMenu->Append(EXIT_ID, _T("E&xit"));

  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _T("&File"));
  
  SetMenuBar(menuBar);
  CreateStatusBar(3); 
  oldWidth = 0;
  oldHeight = 0;
  loadedImage = 0;
  resetImage = 0;
  undoImage = 0;

/* initialise the variables that we added */
  imgWidth = imgHeight = 0;
  stuffToDraw = 0;

  roiSet = false;
  startX = startY = endX = endY = 0;
}

MyFrame::~MyFrame(){
/* release resources */
  if(loadedImage){
    loadedImage->Destroy();
    loadedImage = 0;
  }

  if(resetImage){
    resetImage->Destroy();
    resetImage = 0;
  }

    if(undoImage){
    undoImage->Destroy();
    undoImage = 0;
  }

}

void MyFrame::OnOpenFile(wxCommandEvent & event){
  wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
  if(openFileDialog->ShowModal() == wxID_OK){
    wxString filename = openFileDialog->GetFilename();
    wxString path = openFileDialog->GetPath();
    printf("Loading image form file...");    
    loadedImage = new wxImage(path); //Image Loaded form file 
    resetImage = new wxImage(path);
    undoImage = new wxImage(path);

    if(loadedImage->Ok()){
      stuffToDraw = ORIGINAL_IMG;    // set the display flag
      printf("Done! \n");    
    }
    else {
      printf("error:...");
      loadedImage->Destroy();
      loadedImage = 0;
      resetImage->Destroy();
      resetImage = 0;
      undoImage->Destroy();
      undoImage = 0;
    }
    Refresh();
  }    
}


//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE ABOVE--------------------//
//-----------------------------------------------------------//
//###########################################################//


//INVERT IMAGE
void MyFrame::OnInvertImage(wxCommandEvent & event){

  printf("Inverting...");
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());

SavePreviousState();

if(!roiSet){ // if roi is not set yet
  for( int i=0;i<imgWidth;i++)
   for(int j=0;j<imgHeight;j++){
    loadedImage->SetRGB(i,j,255-loadedImage->GetRed(i,j), 
      255-loadedImage->GetGreen(i,j),
      255-loadedImage->GetBlue(i,j));
  }
} else {// if roi has been set
  for( int i=startX;i<endX;i++)
   for(int j=startY;j<endY;j++){
    loadedImage->SetRGB(i,j,255-loadedImage->GetRed(i,j), 
      255-loadedImage->GetGreen(i,j),
      255-loadedImage->GetBlue(i,j));
}
  printf(" Finished inverting.\n");
  Refresh();
}
}

// Image shifting ------ Lab 5 ------
void MyFrame::ImageShifting(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  unsigned int r,g,b;

  int shiftValue;

  cout << "Please enter a shift value - integer between -255 and 255: ";
  cin >> shiftValue;

  for( int i=0;i<imgWidth;i++)
   for(int j=0;j<imgHeight;j++){

	// get the rgb values and apply shifting

     int r = loadedImage->GetRed(i,j)+shiftValue;   
     int g = loadedImage->GetGreen(i,j)+shiftValue; 
     int b = loadedImage->GetBlue(i,j)+shiftValue; 

  // if value is less than 0, set it to 0

     if(r<0) r = 0;   
     if(g<0) g = 0;
     if(b<0) b = 0;


  // if value is more than 255, set it to 255

     if(r>255) r = 255;
     if(g>255) g = 255;
     if(b>255) b = 255;

     loadedImage->SetRGB(i,j,r,g,b); 
   }

   printf(" Finished image shifting.\n");
   Refresh();
 }

// Image convolution ------ Lab 5 ------

// Convolution function
 void MyFrame::Convolution( double multiplier, double filter[][3]){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxImage *myImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  for( int x=1;x<imgWidth;x++){
    for(int y=1;y<imgHeight;y++){

      double red = 0.0;
      double green = 0.0;
      double blue = 0.0;

      for(int i=-1; i<=1; i++)  {
        for(int j=-1; j<=1; j++)  {
          int iy = i+y;
          int jx = j+x;

          int rTemp = myImage->GetRed(jx, iy);
          int gTemp = myImage->GetGreen(jx, iy);
          int bTemp = myImage->GetBlue(jx, iy);

          red = red + rTemp * filter[j+1][i+1] * multiplier;
          green = green + gTemp * filter[j+1][i+1] * multiplier;
          blue = blue + bTemp * filter[j+1][i+1] * multiplier;
        }
      }

//cout << "red" <<  red << endl;

      if(red>255) red=255;
      if(green>255) green=255;
      if(blue>255) blue=255;

      if(red<0) red=0;
      if(green<0) green=0;
      if(blue<0) blue=0;

      loadedImage->SetRGB(x,y,red,green,blue);
    }
  }
  printf("Finished Convolution.\n");
  Refresh();
}

// Smoothing - Box averaging filter
void MyFrame::BoxAverage(wxCommandEvent & event){

  double multiplier = 1.0/9.0;

  double filter[3][3] =  { 
    1, 1, 1,
    1, 1, 1,
    1, 1, 1,

  };
  Convolution(multiplier, filter);
}

// Smoothing - weighted averaging filter
void MyFrame::WeightedAverage(wxCommandEvent & event){

  double multiplier = 1.0/16.0;

  double filter[3][3] =  { 
    1, 2, 1,
    2, 4, 2,
    1, 2, 1,

  };

  Convolution(multiplier, filter);

}

// Edge detection ------ Lab 5 ------

void MyFrame::Sobel(wxCommandEvent & event){

// sobel matrix
  double filterX[3][3] =  { 
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
  };

  double filterY[3][3] =  { 
    -1, -2, -1,
    0, 0, 0,
    1, 2, 1
  };

EdgeDetection(filterX, filterY);

}

void MyFrame::Robert(wxCommandEvent & event){

  // robert matrix
    double filterX[3][3] =  { 
    0, 0, 0,
    0, 0, -1,
    0, 1, 0
  };

  double filterY[3][3] =  { 
    0, 0, 0,
    0, -1, 0,
    0, 0, 1
  };

  EdgeDetection(filterX, filterY);

}

// Edge detection function
void MyFrame::EdgeDetection(double filterX[][3], double filterY[][3]){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxImage *myImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();
  int tempX, tempY, s;

  for( int x=1;x<imgWidth;x++){
    for(int y=1;y<imgHeight;y++){

      int tempVal = 0;
      tempX = 0;
      tempY = 0;

       for(int i=-1; i<=1; i++)  {
        for(int j=-1; j<=1; j++)  {

          int iy = i + y;
          int jx = j + x;

          int r = myImage->GetRed(jx, iy);
          int g = myImage->GetGreen(jx, iy);
          int b = myImage->GetBlue(jx, iy);

          tempVal = (r+g+b)/3;
          tempX = tempX + tempVal * filterX[j+1][i+1];
          tempY = tempY + tempVal * filterY[j+1][i+1];
        }
      }
    
    // absolute value conversion
    s = abs(tempX) + abs(tempY);
    if(s>255) s=255;
    if(s<0) s=0;

    loadedImage->SetRGB(x,y,s,s,s) ;
  }
}
printf(" Finished Edge detection.\n");
Refresh();
}

// Order-Statistics filtering ------ Lab 6 ------

// Add salt and pepper noise
void MyFrame::AddRandomNoise(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  double random;  
  int r, g, b;

if(!roiSet){ 
  for (int x = 0; x <= imgWidth; x++){
    for (int y = 0; y <= imgHeight; y++){       

      random = ((double) rand() / (RAND_MAX));

      if(random < 0.05){
        loadedImage->SetRGB(x, y, 0, 0, 0);
      }
      else if(random > 0.95){
        loadedImage->SetRGB(x, y, 255, 255, 255);
      }
    }
  }
 } else {

  for (int x = startX; x <= endX; x++){
    for (int y = startY; y <= endY; y++){       

      random = ((double) rand() / (RAND_MAX));

      if(random < 0.05){
        loadedImage->SetRGB(x, y, 0, 0, 0);
      }
      else if(random > 0.95){
        loadedImage->SetRGB(x, y, 255, 255, 255);
      }
    }
  }
 } 
  printf(" Finished adding salt and pepper noise.\n");
  Refresh();
}

// Max filter
void MyFrame::MaxFilter(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxImage *myImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  for (int x = 0; x <= imgWidth ; x++){
    for (int y = 0; y <= imgHeight ; y++){

      int r = (int) myImage->GetRed(x,y);
      int g = (int) myImage->GetGreen(x, y);
      int b = (int) myImage->GetBlue(x, y);

      int av = (r+g+b)/3;
      if(av == 0){
        vector<int> neigh = GetNeighbors(x, y, 0, myImage);
        sort(neigh.begin(), neigh.end());

        int max = neigh[neigh.size()-1];
        loadedImage->SetRGB(x, y, max, max, max);
      } 
    }
  }
  printf(" Finished MaxFilter.\n");
  Refresh();
}

void MyFrame::MinFilter(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxImage *myImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  for (int x = 0; x <= imgWidth ; x++){
    for (int y = 0; y <= imgHeight ; y++){

      int r = (int) loadedImage->GetRed(x,y);
      int g = (int) loadedImage->GetGreen(x, y);
      int b = (int) loadedImage->GetBlue(x, y);

      int av = (r+g+b)/3;
      if(av == 255){
        vector<int> neigh = GetNeighbors(x, y, 0,myImage);
        sort(neigh.begin(), neigh.end());

        int min = neigh[0];
        loadedImage->SetRGB(x, y, min, min, min);
      } 
    }
  }
  printf(" Finished MinFilter.\n");
  Refresh();
}

vector<int> MyFrame::GetNeighbors(int x, int y, int p, wxImage *myImage){

  vector<int> neighbors;
  for(int i=-1; i<=1; i++)  {
    for(int j=-1; j<=1; j++)  {
      int iy = i + y;
      int jx = j + x;
      if(p == 0){
        int r = (int) myImage->GetRed(jx, iy);
        int g = (int) myImage->GetGreen(jx, iy);
        int b = (int) myImage->GetBlue(jx, iy);
        neighbors.push_back((r+g+b)/3);
      }
      else if(p == 1) neighbors.push_back((int) myImage->GetRed(jx, iy));
      else if(p == 2) neighbors.push_back((int)myImage->GetGreen(jx, iy));
      else neighbors.push_back((int)myImage->GetBlue(jx, iy));
    }
  }
  return neighbors;
}

// Enhancement in spatial domain
// Point Processing ------ Lab 7 ------
// Negative linear function
void MyFrame::NegativeLinearTransform(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();
  for( int i=0;i<=imgWidth;i++)
   for(int j=0;j<=imgHeight;j++){
    loadedImage->SetRGB(i,j,255-loadedImage->GetRed(i,j), 
      255-loadedImage->GetGreen(i,j),
      255-loadedImage->GetBlue(i,j));
  }

  printf(" Finished negative.\n");
  Refresh();
}

// Logarithmic function
void MyFrame::LogarithmicFunction(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();
  int constant = 0;
  int largestSoFar =0;
  logTable.clear();

  for( int i=0;i<=imgWidth;i++){
    for(int j=0;j<=imgHeight;j++){
      int r = (int) loadedImage->GetRed(i, j);
      int g = (int) loadedImage->GetGreen(i, j);
      int b = (int) loadedImage->GetBlue(i, j);

      int val = (r+g+b)/3;

      if(val > largestSoFar){largestSoFar = val;}
    }
  }
  constant = 255/log10(1+largestSoFar);
  for( int i=0;i<256;i++){
    double tempVal = constant * log10(i+1);

    if(tempVal > 255) tempVal = 255;

    int tempInt = tempVal;  
    logTable.push_back(tempInt);
  }
  for( int i=0;i<256;i++){

    constant = 1/log2(1+i);
    double tempVal = constant * log2(((double) i/255)+1);

    if(tempVal > 1){
      tempVal = 1;
    }
    int tempInt = tempVal * 255; 

    logTable.push_back(tempInt);
    if(tempInt > largestSoFar){
      largestSoFar = tempInt;
    }
  }
  for( int i=0;i<=imgWidth;i++){
   for(int j=0;j<=imgHeight;j++){
    int r = (int) loadedImage->GetRed(i, j);
    int g = (int) loadedImage->GetGreen(i, j);
    int b = (int) loadedImage->GetBlue(i, j);

    int val = (r+g+b)/3;

    loadedImage->SetRGB(i,j, logTable[val], logTable[val], logTable[val]);
  }
}
printf("Finished Logarithmic function.\n");
Refresh();
}

// Power law function
// with c = 1

void MyFrame::PowerLawFunction(wxCommandEvent & event){ 

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  double gamma;
  double c;

  cout << "Set gamma value from 0.01 to 25: ";
  cin >> gamma;
  cout << "Set constant: ";
  cin >> c;

  powerTable.clear();

  for( int i=0;i<256;i++){   
    double tempVal = c*pow(((double) i/255), gamma);

    if(tempVal > 1)
      tempVal = 1;

    powerTable.push_back(tempVal*255); 
  }

  for( int i=0;i<=imgWidth;i++){
   for(int j=0;j<=imgHeight;j++){
    int r = (int) loadedImage->GetRed(i, j);
    int g = (int) loadedImage->GetGreen(i, j);
    int b = (int) loadedImage->GetBlue(i, j);
    int p = (r+g+b)/3;
    loadedImage->SetRGB(i,j, powerTable[p], powerTable[p], powerTable[p]);
  }
}
printf("Finished Power Law function.\n");
Refresh();
}

// Histogram Equalisation ------ Lab 8 ------

void MyFrame::FindAndNormalise(wxCommandEvent & event){
  //1) find the histogram by counting the number of pixel values 

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage()); 

  histogram.clear();
  normalisedHistogram.clear();

  for(int i = 0; i<256; i++){
    histogram.push_back(0); 
  }

  for( int i=0;i<=imgWidth;i++){
    for(int j=0;j<=imgHeight;j++){
      histogram[(int) loadedImage->GetRed(i, j)] += 1;
    }
  }
  int s = 0;
  int highest = 0;

  for(int i = 0; i<256; i++){
    s += histogram[i];
    if(histogram[i] > highest){
      highest = histogram[i];
    }
  }

//2) normalise the histogram by dividing by the number of counted pixels

  for(int i = 0; i<256; i++){
    double norm = (double) histogram[i]/(double)s;
    normalisedHistogram.push_back(norm);
  }
  printf("Histogram found and normalised.\n");
}

void MyFrame::Equalise(wxCommandEvent & event){

//3) equalise the histogram, find the corresponding gray levels and 
//then map them to the image

  if(normalisedHistogram.size()>0)
  {
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage()); 

    vector<double> tempVec;

    for(int i = 0; i<256; i++){
      double s = 0.0;
      for(int j = 0; j<=i; j++){
        s+= normalisedHistogram[j];
      }
      tempVec.push_back(s);
    }

    for(int i =0; i<256; i++){
      histogram[i] = tempVec[i]*255;
    }
    for( int i=0;i<=imgWidth;i++){
      for(int j=0;j<=imgHeight;j++){
        int r = (int) loadedImage->GetRed(i, j);
        int g = (int) loadedImage->GetGreen(i, j);
        int b = (int) loadedImage->GetBlue(i, j);

        int val = (r+g+b)/3;

        loadedImage->SetRGB(i,j, histogram[val], histogram[val], histogram[val]);
      }
    }
    printf("Histogram Equalised.\n");
    Refresh();
  } else {
    printf("Please find and normalise a histogram first.\n");
  }

}

// Thresholding ------ Lab 9 ------

// Find mean and standard deviation values
void MyFrame::MeanAndStandardDeviation(wxCommandEvent & event){

  if(normalisedHistogram.size()>0)
  {
    double mean = 0.0;
    for(int i = 0; i<256; i++){
      mean += normalisedHistogram[i]*i;
    }
    cout << "Mean deviation: " << mean << endl;

    double variance = 0.0;
    for(int i = 0; i<256; i++){
      variance += pow((i-mean), 2) * normalisedHistogram[i];
    }

    double standard = sqrt(variance);
    cout << "Standard deviation: "<< standard << endl;
  }
  else
    { printf("Please find and normalise a histogram first.\n");}
}

// Simple thresholding
void MyFrame::SimpleThresholding(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  SavePreviousState();

  thresholdTable.clear();

  int thresh;
  cout << "Enter threshold value between 0 and 255: ";
  cin >> thresh;

  for( int i=0;i<256;i++){
    int x;

    if (i<thresh)
      x = 0;

    else
      x = 255;

    thresholdTable.push_back(x); 
  }

  for( int i=0;i<=imgWidth;i++){
   for(int j=0;j<=imgHeight;j++){
    int r = (int) loadedImage->GetRed(i, j);
    int g = (int) loadedImage->GetGreen(i, j);
    int b = (int) loadedImage->GetBlue(i, j);

    int val = (r+g+b)/3;

    loadedImage->SetRGB(i,j, thresholdTable[val], thresholdTable[val], thresholdTable[val]);
  }
}
printf(" Finished Simple thresholding.\n");
Refresh();
}


void MyFrame::Reset(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());

  for( int i=0;i<imgWidth;i++){
   for(int j=0;j<imgHeight;j++){

    int r = (int) resetImage->GetRed(i, j);
    int g = (int) resetImage->GetGreen(i, j);
    int b = (int) resetImage->GetBlue(i, j);

    loadedImage->SetRGB(i,j, r, g, b);
  }
}
Refresh();

}

void MyFrame::SavePreviousState(){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());

  for( int i=0;i<imgWidth;i++){
   for(int j=0;j<imgHeight;j++){

    int r = (int) loadedImage->GetRed(i, j);
    int g = (int) loadedImage->GetGreen(i, j);
    int b = (int) loadedImage->GetBlue(i, j);

    undoImage->SetRGB(i,j, r, g, b);
  }
}
Refresh();

}

void MyFrame::Undo(wxCommandEvent & event){

  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());

  for( int i=0;i<imgWidth;i++){
   for(int j=0;j<imgHeight;j++){

    int r = (int) undoImage->GetRed(i, j);
    int g = (int) undoImage->GetGreen(i, j);
    int b = (int) undoImage->GetBlue(i, j);

    loadedImage->SetRGB(i,j, r, g, b);
  }
}
Refresh();
}

void MyFrame::SetROI(wxCommandEvent & event){

  roiSet = true;

 cout << "Set startX: ";
  cin >> startX;

   cout << "Set startY: ";
  cin >> startY;

   cout << "Set endX: ";
  cin >> endX;

   cout << "Set endY: ";
  cin >> endY;

  cout << "ROI has been set!\n";

}

void MyFrame::UnsetROI(wxCommandEvent & event){
  roiSet = false;
  cout << "ROI is no longer set!\n";
}

//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE BELOW--------------------//
//-----------------------------------------------------------//
//###########################################################//


//IMAGE SAVING
void MyFrame::OnSaveImage(wxCommandEvent & event){

  printf("Saving image...");
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());

  loadedImage->SaveFile(wxT("Saved_Image.bmp"), wxBITMAP_TYPE_BMP);

  printf("Finished Saving.\n");
}


void MyFrame::OnExit (wxCommandEvent & event){
  Close(TRUE);
}


void MyFrame::OnPaint(wxPaintEvent & event){
  wxPaintDC dc(this);
  int cWidth, cHeight;  
  GetSize(&cWidth, &cHeight);
  if ((back_bitmap == NULL) || (oldWidth != cWidth) || (oldHeight != cHeight)) {
    if (back_bitmap != NULL)
      delete back_bitmap;
    back_bitmap = new wxBitmap(cWidth, cHeight);
    oldWidth = cWidth;
    oldHeight = cHeight;
  }
  wxMemoryDC *temp_dc = new wxMemoryDC(&dc);
  temp_dc->SelectObject(*back_bitmap);
  // We can now draw into the offscreen DC...
  temp_dc->Clear();
  if(loadedImage)
    temp_dc->DrawBitmap(wxBitmap(*loadedImage), 0, 0, false);//given bitmap xcoord y coord and transparency

  switch(stuffToDraw){
   case NOTHING:
   break;
   case ORIGINAL_IMG:
       bitmap.CleanUpHandlers; // clean the actual image header
       bitmap = wxBitmap(*loadedImage); // Update the edited/loaded image
       break;
     }

// update image size
     imgWidth  = (bitmap.ConvertToImage()).GetWidth();
     imgHeight = (bitmap.ConvertToImage()).GetHeight();



 temp_dc->SelectObject(bitmap);//given bitmap 

  //end draw all the things
  // Copy from this DC to another DC.
 dc.Blit(0, 0, cWidth, cHeight, temp_dc, 0, 0);
  delete temp_dc; // get rid of the memory DC  
}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
EVT_MENU ( LOAD_FILE_ID,  MyFrame::OnOpenFile)
EVT_MENU ( EXIT_ID,  MyFrame::OnExit)

//###########################################################//
//----------------------START MY EVENTS ---------------------//
//###########################################################// 

EVT_MENU ( RESET_ID, MyFrame::Reset)
EVT_MENU ( UNDO_ID, MyFrame::Undo)

EVT_MENU ( INVERT_IMAGE_ID,  MyFrame::OnInvertImage)
EVT_MENU ( SAVE_IMAGE_ID,  MyFrame::OnSaveImage)
EVT_MENU ( SHIFT_ID,  MyFrame::ImageShifting)

EVT_MENU ( BOXAVERAGE_ID,  MyFrame::BoxAverage) 
EVT_MENU ( WEIGHTEDAVERAGE_ID,  MyFrame::WeightedAverage)

EVT_MENU ( SOBELEDGE_ID,  MyFrame::Sobel) 
EVT_MENU ( ROBERTEDGE_ID,  MyFrame::Robert) 

EVT_MENU ( NOISE_ID,  MyFrame::AddRandomNoise) 
EVT_MENU ( MAX_ID,  MyFrame::MaxFilter)
EVT_MENU ( MIN_ID,  MyFrame::MinFilter)

EVT_MENU ( NEGATIVE_ID,  MyFrame::NegativeLinearTransform)
EVT_MENU ( LOG_ID,  MyFrame::LogarithmicFunction)
EVT_MENU ( POWER_ID,  MyFrame::PowerLawFunction)

EVT_MENU ( EQUALISE_ID,  MyFrame::Equalise)
EVT_MENU ( NORM_ID,  MyFrame::FindAndNormalise)

EVT_MENU ( MEAN_ID,  MyFrame::MeanAndStandardDeviation)
EVT_MENU ( THRESH_ID,  MyFrame::SimpleThresholding)

EVT_MENU ( SETROI_ID, MyFrame::SetROI)
EVT_MENU ( UNSETROI_ID, MyFrame::UnsetROI)

//###########################################################//
//----------------------END MY EVENTS -----------------------//
//###########################################################// 

EVT_PAINT (MyFrame::OnPaint)
END_EVENT_TABLE()
