#include <vector>
class BasicApplication : public wxApp {
 public:
    virtual bool OnInit();
};


class MyFrame : public wxFrame {    
 protected:
    wxMenuBar  *menuBar; //main menu bar
    wxMenu     *fileMenu; //file menu
    wxBitmap *back_bitmap; // offscreen memory buffer for drawing
    wxToolBar *toolbar; //tollbar not necessary to use
    int oldWidth, oldHeight; //save old dimensions

    wxBitmap bitmap;  //structure for the edited image
    wxImage *loadedImage; //image loaded from file
    wxImage *resetImage; //reset image loaded from file
    wxImage *undoImage; //undo image 
    int imgWidth, imgHeight; //image dimensions
    int stuffToDraw; 

    bool roiSet;
    int startX, startY, endX, endY;

    void OnInvertImage(wxCommandEvent & event);
    void OnSaveImage(wxCommandEvent & event);
    void ImageShifting(wxCommandEvent & event);

    void Convolution(double multiplier, double filterX[][3]);

    void BoxAverage(wxCommandEvent & event);
    void WeightedAverage(wxCommandEvent & event);

    void EdgeDetection(double filterX[][3], double filterY[][3]);
    void Sobel(wxCommandEvent & event);
    void Robert(wxCommandEvent & event);

    void AddRandomNoise(wxCommandEvent & event);
    void MaxFilter(wxCommandEvent & event);
    void MinFilter(wxCommandEvent & event);

    void NegativeLinearTransform(wxCommandEvent & event);
    void LogarithmicFunction(wxCommandEvent & event);
    void PowerLawFunction(wxCommandEvent & event);

    void FindAndNormalise(wxCommandEvent & event);
    void Equalise(wxCommandEvent & event);

    void MeanAndStandardDeviation(wxCommandEvent & event);
    void SimpleThresholding(wxCommandEvent & event);

    void Reset(wxCommandEvent & event); 
    void SavePreviousState();
    void Undo(wxCommandEvent & event); 

    void SetROI(wxCommandEvent & event); 
    void UnsetROI(wxCommandEvent & event); 

    std::vector<int> GetNeighbors(int x, int y, int p, wxImage *myImage);
    std::vector<int> logTable;
    std::vector<int> powerTable;
    std::vector<int> histogram;
    std::vector<double> normalisedHistogram;  
    std::vector<int> thresholdTable;

 public:
    MyFrame(const wxString title, int xpos, int ypos, int width, int height);
    virtual ~MyFrame();
    
    void OnExit(wxCommandEvent & event);       
    void OnOpenFile(wxCommandEvent & event);       
    void OnPaint(wxPaintEvent & event);	
    
    DECLARE_EVENT_TABLE()
	
};

enum { NOTHING = 0,
       ORIGINAL_IMG,
};

enum { EXIT_ID = wxID_HIGHEST + 1,      
       LOAD_FILE_ID,
       INVERT_IMAGE_ID,
       SAVE_IMAGE_ID,
       SHIFT_ID, 
       BOXAVERAGE_ID,
       WEIGHTEDAVERAGE_ID,
       SOBELEDGE_ID,
       ROBERTEDGE_ID,
       NOISE_ID,
       MAX_ID,
       MIN_ID,
       NEGATIVE_ID,
       POWER_ID,
       LOG_ID,
       NORM_ID,
       EQUALISE_ID,
       MEAN_ID,
       THRESH_ID,
       RESET_ID,
       UNDO_ID,
       SETROI_ID,
       UNSETROI_ID,
};

