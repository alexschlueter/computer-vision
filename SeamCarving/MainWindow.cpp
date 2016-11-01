#include "MainWindow.hpp"
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    /* initialisiere die UI Komponeten */
    setupUi();
}

MainWindow::~MainWindow()
{
    /* loesche die UI Komponeten */
    delete centralWidget;    
    
    /* schliesse alle offenen Fenster */
    cv::destroyAllWindows();
}

/* Methode oeffnet ein Bild und zeigt es in einem separaten Fenster an */
void MainWindow::on_pbOpenImage_clicked()
{
    /* oeffne Bild mit Hilfe eines Dateidialogs */
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image...", QString(), QString("Images *.png *.jpg *.tiff *.tif"));
    
    /* wenn ein gueltiger Dateipfad angegeben worden ist... */
    if(!imagePath.isNull() && !imagePath.isEmpty())
    {
        /* ...lese das Bild ein */
        cv::Mat img = ImageReader::readImage(QtOpencvCore::qstr2str(imagePath));
        
        /* wenn das Bild erfolgreich eingelesen worden ist... */
        if(!img.empty())
        {
            /* ... merke das Originalbild ... */
            originalImage = img;
            
            /* ... aktiviere das UI ... */
            enableGUI();
            
            /* ... zeige das Originalbild in einem separaten Fenster an */
            cv::imshow("Original Image", originalImage); 
        }
        else
        {
            /* ...sonst deaktiviere das UI */
            disableGUI();
        }
    }
}

void MainWindow::on_pbComputeSeams_clicked()
{
    /* Anzahl der Spalten, die entfernt werden sollen */
    int colsToRemove = sbCols->value();
    
    /* Anzahl der Zeilen, die entfernt werden sollen */
    int rowsToRemove = sbRows->value();
    
    calcL1Energy();
    // cv::imshow("L1 Energy (Sobel)", energyImage);
    buildDPMat();
}

void MainWindow::on_pbRemoveSeams_clicked()
{
    std::cout << originalImage.type() << std::endl;
    for (int i = 0; i < sbCols->value(); ++i)
    {
        on_pbComputeSeams_clicked();
        resultImage.create(originalImage.rows, originalImage.cols-1, originalImage.type());
        std::cout << "create" << std::endl;
        cv::Point minLoc;
        cv::minMaxLoc(DPMat.row(DPMat.rows-1), nullptr, nullptr, &minLoc);
        std::cout << "minmax" << std::endl;
        int col = minLoc.x;
        for (int r = DPMat.rows-1; r >= 0; --r)
        {
            // std::cout << r << std::endl;
            // originalImage.at<cv::Vec3b>(r, col) = 0;
            auto row = originalImage.ptr<cv::Vec3b>(r);
            auto resRow = resultImage.ptr<cv::Vec3b>(r);
            for (int c = 0; c < col; ++c)
            {
                resRow[c] = row[c];
            }
            for (int c = col+1; c < originalImage.cols; ++c)
            {
                resRow[c-1] = row[c];
            }

            col += DPPointers.at<char>(r-1, col);
            if (r==0) std::cout << "r=0 " << r << " " << col << std::endl;
        }
        if (i != sbCols->value()-1) cv::swap(originalImage, resultImage);
    }
    cv::imshow("result", resultImage);
}

void MainWindow::setupUi()
{
    /* Boilerplate code */
    /*********************************************************************************************/
    resize(129, 211);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setMinimumSize(QSize(129, 211));
    setMaximumSize(QSize(129, 211));
    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QString("centralWidget"));
    
    horizontalLayout = new QHBoxLayout(centralWidget);
    verticalLayout = new QVBoxLayout();
    
    pbOpenImage = new QPushButton(QString("Open Image"), centralWidget);
    verticalLayout->addWidget(pbOpenImage);
    
    
    verticalLayout_3 = new QVBoxLayout();
    lCaption = new QLabel(QString("Remove"), centralWidget);
    lCaption->setEnabled(false);
    verticalLayout_3->addWidget(lCaption);
    
    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString("horizontalLayout_3"));
    lCols = new QLabel(QString("Cols"), centralWidget);
    lCols->setEnabled(false);
    lRows = new QLabel(QString("Rows"), centralWidget);
    lRows->setEnabled(false);
    horizontalLayout_3->addWidget(lCols);
    horizontalLayout_3->addWidget(lRows);
    verticalLayout_3->addLayout(horizontalLayout_3);
    
    horizontalLayout_2 = new QHBoxLayout();
    sbCols = new QSpinBox(centralWidget);
    sbCols->setEnabled(false);
    horizontalLayout_2->addWidget(sbCols);
    sbRows = new QSpinBox(centralWidget);
    sbRows->setEnabled(false);
    horizontalLayout_2->addWidget(sbRows);
    verticalLayout_3->addLayout(horizontalLayout_2);
    verticalLayout->addLayout(verticalLayout_3);
    
    pbComputeSeams = new QPushButton(QString("Compute Seams"), centralWidget);
    pbComputeSeams->setEnabled(false);
    verticalLayout->addWidget(pbComputeSeams);
    
    pbRemoveSeams = new QPushButton(QString("Remove Seams"), centralWidget);
    pbRemoveSeams->setEnabled(false);
    verticalLayout->addWidget(pbRemoveSeams);
    
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);
    horizontalLayout->addLayout(verticalLayout);
    
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);
    setCentralWidget(centralWidget);
    /*********************************************************************************************/
    
    
    /* Verbindung zwischen den Buttonklicks und den Methoden, die beim jeweiligen Buttonklick ausgefuehrt werden sollen */
    connect(pbOpenImage,    &QPushButton::clicked, this, &MainWindow::on_pbOpenImage_clicked);  
    connect(pbComputeSeams, &QPushButton::clicked, this, &MainWindow::on_pbComputeSeams_clicked); 
    connect(pbRemoveSeams,  &QPushButton::clicked, this, &MainWindow::on_pbRemoveSeams_clicked);
}

void MainWindow::enableGUI()
{
    lCaption->setEnabled(true);
    
    lCols->setEnabled(true);
    lRows->setEnabled(true);
    
    sbCols->setEnabled(true);
    sbRows->setEnabled(true);
    
    pbComputeSeams->setEnabled(true);
    pbRemoveSeams->setEnabled(true);
    
    sbRows->setMinimum(0);
    sbRows->setMaximum(originalImage.rows);
    sbRows->setValue(2);
    
    sbCols->setMinimum(0);
    sbCols->setMaximum(originalImage.cols);
    sbCols->setValue(2);
}

void MainWindow::disableGUI()
{
    lCaption->setEnabled(false);
    
    lCols->setEnabled(false);
    lRows->setEnabled(false);
    
    sbCols->setEnabled(false);
    sbRows->setEnabled(false);
    
    pbComputeSeams->setEnabled(false);
    pbRemoveSeams->setEnabled(false);
}

void MainWindow::calcL1Energy()
{
    cv::Mat grayscale, diffx, diffy, out;

    // cv::Sobel(originalImage, diffx, -1, 1, 0, 3);
    // cv::Sobel(originalImage, diffy, -1, 0, 1, 3);
    // energyImage = cv::abs(diffx) + cv::abs(diffy);
    // cv::cvtColor(energyImage, out, CV_BGR2GRAY);
    // cv::imshow("after", out);


    cv::cvtColor(originalImage, grayscale, CV_BGR2GRAY);
    cv::Sobel(grayscale, diffx, -1, 1, 0, 3);
    cv::Sobel(grayscale, diffy, -1, 0, 1, 3);
    // cv::spatialGradient(originalImage, diffx, diffy);
    energyImage = cv::abs(diffx) + cv::abs(diffy);
    std::cout << energyImage.type() << std::endl;
    // cv::imshow("vorscale", grayscale);
    // cv::normalize(energyImage, energyImage, 0, 255, cv::NORM_MINMAX);
}

void MainWindow::buildDPMat()
{
    // auto firstRow = energyImage.ptr<float>(0);
    // for (c = 0; c < energyImage.cols; ++c)
    // {
    //     firstRow[c] = 
    // }
    std::cout << "lol" << std::endl;
    // energyImage = (cv::Mat_<uchar>(5, 5) << 1,2,3,4,5,6,5,2,4,1,3,4,6,2,1,6,3,2,7,15,3,2,44,32,1);
    // std::cout << DPMat << std::endl;
    std::cout << "dpmat" << std::endl;
    // std::cout << energyImage << std::endl;
    std::cout << "energymat" << std::endl;
    DPPointers.create(energyImage.rows-1, energyImage.cols, CV_8SC1);
    std::cout << "postassign" << energyImage.rows << energyImage.cols << std::endl;
    DPMat.create(energyImage.rows, energyImage.cols+2, energyImage.type());
    std::cout << "preborder" << std::endl;
    cv::copyMakeBorder(energyImage, DPMat, 0, 0, 1, 1, cv::BORDER_CONSTANT, 255);
    // DPMat = cv::Mat::zeros(energyImage.size(), energyImage.type());
    // energyImage.row(0).copyTo(DPMat.row(0));
    std::cout << "preloop" << std::endl;
    for (int r = 1; r < DPMat.rows; ++r)
    {
        auto row = DPMat.ptr<uchar>(r);
        auto prow = DPPointers.ptr<char>(r-1);
        for (int c = 1; c < DPMat.cols-1; ++c)
        {
            // std::cout << r << " " << c << std::endl;
            double min;
            cv::Point minLoc;
            cv::minMaxLoc(DPMat(cv::Rect(c-1, r-1, 3, 1)), &min, nullptr, &minLoc);
            row[c] += min;
            prow[c-1] = minLoc.x-1;
        }
    }
    std::cout << "postloop" << std::endl;
    // std::cout << mask << std::endl << mask.type() << std::endl;
    // cv::erode(DPMat, DPMat, mask, cv::Point(1, 1), DPMat.rows - 1);
    // std::cout << DPMat << std::endl;
    // std::cout << DPPointers << std::endl;

    cv::Point minLoc;
    cv::minMaxLoc(DPMat.row(DPMat.rows-1), nullptr, nullptr, &minLoc);
    int col = minLoc.x;
    for (int r = DPMat.rows-2; r >= 0; --r)
    {
        col += DPPointers.at<char>(r, col);
        originalImage.at<cv::Vec3b>(r, col) = 0;
        energyImage.at<uchar>(r, col) = 255;
    }
    std::cout << "postloop" << std::endl;
    // std::cout << energyImage << std::endl;
    // static int j=0;
    // cv::imshow("seam"+std::to_string(j), originalImage);
    // cv::imshow("seame"+std::to_string(j++), energyImage);
}
