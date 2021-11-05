/**
 * @authors Yoan Laurain ; Hugo Carricart ; Nathan Lesourd
 * @brief Source Code de MainWindow
 * @file mainwindow.cpp
 * @date 26/10/2021
 * @version 0.5
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ChessBoard.h"
#include "Piece.h"
#include "Player.h"
#include <iostream>
#include <assert.h>
#include "mainwindow.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <vector>
#include <string>
#include <sstream>
#include <QStringListModel>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>

using namespace std;

/**
 * @brief Builder of the game page, initialize the game
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect ( timer , SIGNAL( timeout() ), this, SLOT( SetTimer() ) );

    assert( playerWhite.PlacePieces( e ) );
    assert( playerBlack.PlacePieces( e ) );

    // Initialisation image pour le premier coup
    QPixmap monImage(":/img/white/assets/white/pawn.png");
    ui->TourLabel->setAlignment(Qt::AlignCenter);
    ui->TourLabel->setPixmap(monImage);

    ui->view_PionNoir->setIconSize( QSize( 43 , 43 ) );
    ui->view_PionBlanc->setIconSize( QSize( 43 , 43 ) );
    ui->view_Histo->setIconSize(QSize(20,20));
    ui->view_Histo_2->setIconSize(QSize(20,20));

    // Initialisation du premier visuel de la matrice
    this->RefreshMatrix(this);

    connect(ui->actionRegles, SIGNAL(triggered()), this, SLOT(ActRegle()));
    connect(ui->actionDark, SIGNAL(triggered()), this, SLOT(ActDarkMode()));
    connect(ui->actionLight, SIGNAL(triggered()), this, SLOT(ActLightMode()));
    connect(ui->actionDocumentation_Code_Source, SIGNAL(triggered()), this , SLOT(ActDocumentation()));

    timer->start();
}

/**
 * @brief Game Page Destroyer
 */
MainWindow::~MainWindow() { delete ui; }

/**
 * @brief Refreshes the game board after each move
 * @param parent
 */
void
MainWindow::RefreshMatrix(QWidget *parent)
{
    model = new QStandardItemModel(8,8,parent) ;

    for ( int i = 0 ; i < 8 ; i++ )
        for ( int j = 0 ; j < 8 ; j++ )
        {
            if ( e.GetPiece( j + 1 , i + 1 ) != nullptr )
                SetPicture( QPixmap ( ConvertToChar( e.GetPiece( j + 1 , i + 1 )->GetPath() ) ), i , j , model);

            QColor myColor = ( i % 2 == j % 2 ?  QColor (103,159,90, 150) : QColor (225, 206, 154, 150) ) ;

            SetColorBackGround( myColor, i , j , model);

            if (  e.GetPiece(j+1, i+1) != nullptr && e.GetPiece( j + 1 , i + 1 )->GetIsEchec() )
                SetColorBackGround( QColor ( "darkRed" ), i , j , model);
        }

    QPixmap monImage = ( whitePlay ? QPixmap ( ":/img/white/assets/white/pawn.png" ) : QPixmap ( ":/img/black/assets/black/pawn.png" ) );

    ui->TourLabel->setAlignment( Qt::AlignCenter );
    ui->TourLabel->setPixmap( monImage );

    ui->tableViewEchiquier->setModel( model );
    ui->tableViewEchiquier->setIconSize( QSize( 90 , 90 ) );
}

/**
 * @brief Method called on the click of a cell
 * @param &index -> Index of the selected cell
 */
void
MainWindow::on_tableViewEchiquier_clicked(const QModelIndex &index)
{
    QVariant selectedCell      = model->data( index, Qt::BackgroundRole );
    QColor colorOfSelectedCell = selectedCell.value<QColor>();

    if ( currentPiece != nullptr && currentPiece->GetIsWhite() == whitePlay && colorOfSelectedCell.value() == 255 )
    {
        if ( ( ( whitePlay ) && ( xWhiteKing != index.column()+1 || yWhiteKing != index.row()+1 ) ) || ( !whitePlay ) && ( xBlackKing != index.column() + 1 || yBlackKing != index.row() + 1 ) )
        {
            if ( xBlackKing == index.column()+1 && yBlackKing == index.row()+1 ) xBlackKing = 0;
            if ( xWhiteKing == index.column()+1 && yWhiteKing == index.row()+1 ) xWhiteKing = 0;

            if ( currentPiece->GetX() == xBlackKing && currentPiece->GetY() == yBlackKing )
            {
                xBlackKing = index.column()+1;
                yBlackKing = index.row()+1;
            }
            else if (  currentPiece->GetX() == xWhiteKing && currentPiece->GetY() == yWhiteKing )
            {
                xWhiteKing = index.column()+1;
                yWhiteKing = index.row()+1;
            }

            if (xBlackKing == 0 || xWhiteKing == 0)
            {
                this->RefreshMatrix(this);
                timer->stop();
                EndGameDisplay();
            }

            if ( whitePlay == true)
            {
                if ( e.GetPiece( index.column() + 1 , index.row() + 1 ) != nullptr )
                {
                   picturesEatedWhitePieces.push_back( e.GetPiece( index.column() + 1 , index.row() + 1 )->GetPath() );
                   DisplayEatPieces(picturesEatedWhitePieces,true);
                }
            }
            else if ( e.GetPiece( index.column() + 1 , index.row() + 1 ) != nullptr )
            {
                picturesEatedBlackPieces.push_back( e.GetPiece( index.column() + 1 , index.row() + 1 )->GetPath() );
                DisplayEatPieces(picturesEatedBlackPieces,false);
            }

            AddToHistory(currentPiece, index.column()+1 , index.row()+1);
            e.MovePiece( currentPiece , index.column()+1  , index.row()+1 );

            if ( whitePlay )
            {
               if ( e.GetPiece( xWhiteKing , yWhiteKing )->GetIsEchec() )  e.GetPiece( xWhiteKing , yWhiteKing )->SetIsEchec();
               this->SetColor( currentPiece->DisplayAvailableMovement( e , whitePlay ) );

               if ( this->Echec( xBlackKing , yBlackKing ) )
               {
                   cout << "Echec position xRoi : " << xBlackKing << " Y: " << yBlackKing << endl;
                   bool isEchecMat = this->IsEchecMat( currentPiece->CheckAvailableMovementKing( e , xBlackKing , yBlackKing ) );

                   if ( isEchecMat )
                   {
                       cout << "Echec et mat" << endl;
                       //Fin de partie
                   }
                   e.GetPiece( xBlackKing , yBlackKing )->SetIsEchec();
               }
            }
            else
            {
                if ( e.GetPiece( xBlackKing , yBlackKing )->GetIsEchec() )
                     e.GetPiece( xBlackKing , yBlackKing )->SetIsEchec();

                this->SetColor( currentPiece->DisplayAvailableMovement(e,whitePlay));

                if ( this->Echec( xWhiteKing , yWhiteKing ) )
                {
                    bool isEchecMat = this->IsEchecMat( currentPiece->CheckAvailableMovementKing( e , xWhiteKing , yWhiteKing ) );

                    if ( isEchecMat )
                    {
                        cout << "Echec et mat" << endl;
                    }
                    e.GetPiece( xWhiteKing , yWhiteKing )->SetIsEchec();
                }
            }

            whitePlay = !whitePlay;
            this->RefreshMatrix(this);
        }
    }
    else
    {
        currentPiece = e.GetPiece(  index.column()+1  , index.row()+1 );
        this->RefreshMatrix(this);
        if ( currentPiece != nullptr && currentPiece->GetX() == xWhiteKing && currentPiece->GetY() == yWhiteKing )
        {
            this->SetColor(WithdrawUnAcceptedMoveOfKing ( currentPiece->DisplayAvailableMovement( e , whitePlay ) ));
        }
        else
        {
           if ( currentPiece != nullptr ) this->SetColor( currentPiece->DisplayAvailableMovement(e,whitePlay) );
        }
    }
}

/**
 * @brief Assigns the colors light blue or pink for the valid cells retrieved by the list as a parameter
 * @param list <string> values ​​-> List of all the coordinates of the cells valid for the movements of the selected part.
 *        The coordinates entered are in the format: 'x-y-bool' where x corresponds to the column of the cell, y corresponds to the row
 *        of the cell and bool corresponds to the attribution of the color true -> blue, false -> pink
 */
void
MainWindow::SetColor(list<string>values)
{
   for (string coordonees : values)
   {
       std::vector<std::string> seglist = SplitString( coordonees, '-');

       try {
           QColor color = ( seglist.at(2) == "true" ? QColor (255,0,255, 255) : QColor (116,208,255, 255) );
           QModelIndex index = model->index( std::stoi( seglist.at(1) ) , std::stoi( seglist.at(0) ) ,QModelIndex());
           model->setData(index, QColor( color ), Qt::BackgroundRole  );
       }  catch (...) {}
   }
   ui->tableViewEchiquier->setModel(model);
}

/**
 * @brief Check if the king's square is red so in check
 * @param x -> coordinate of the king's column
 * @param y -> coordinate of the king's line
 * @return bool -> Determines if the box is red
 */

list<string>
MainWindow::WithdrawUnAcceptedMoveOfKing(list<string> values)
{
    int line = 0;
    for (string coordonees : values)
    {
        std::vector<std::string> seglist = SplitString( coordonees, '-');
        try {
            int xCoordonees = std::stoi( seglist.at(0) ) + 1;
            int yCoordonees = std::stoi( seglist.at(1) ) + 1;

            for ( int i = 0 ; i < 8 ; i++)
            {
                for ( int j = 0 ; j < 8 ; j++)
                {
                   if ( e.GetPiece( i , j ) != nullptr)
                   {
                       if ( e.GetPiece( i , j )->GetIsWhite() != whitePlay)
                       {
                           list<string> valuesPiece = e.GetPiece( i , j )->DisplayAvailableMovement( e, whitePlay);
                           cout << valuesPiece.size() << endl;
                           for (string coordonees2 : valuesPiece)
                           {
                               std::vector<std::string> seglistPiece = SplitString( coordonees2, '-');
                               try {
                                   cout << "X : " << seglist.at(0) << " Y: " << seglist.at(1) << endl;
                                   if ( std::stoi( seglist.at(0)  ) > 0 && std::stoi( seglist.at(1) ) > 0 )
                                   {
                                       int xCoordoneesPiece = std::stoi( seglist.at(0) ) + 1;
                                       int yCoordoneesPiece = std::stoi( seglist.at(1) ) + 1;
                                       if ( xCoordonees == xCoordoneesPiece && yCoordonees == yCoordoneesPiece )
                                       {
                                            values.remove(coordonees);
                                       }
                                   }
                               }  catch (...) {}
                           }
                       }
                   }
                }
            }
        }  catch (...) {}
        line++;
    }
    cout << "----------" << endl;
    for (string coordonees3 : values)
    {
        try {
            std::vector<std::string> seglist = SplitString( coordonees3, '-');
            int xtemp = std::stoi( seglist.at(0) ) + 1;
            int ytemp = std::stoi( seglist.at(1) ) + 1;
            cout << "X : " << xtemp << " Y: " << ytemp << endl;
        }  catch (...) {}
    }
    return values;
}

bool
MainWindow::Echec( int x , int y )
{
    QModelIndex index = model->index( y - 1,x - 1 , QModelIndex() );
    QVariant selectedCell      = model->data( index, Qt::BackgroundRole );
    QColor colorOfSelectedCell = selectedCell.value<QColor>();

    return ( colorOfSelectedCell.value() == 255 ? true : false );
}

/**
 * @brief Checks if in the list of valid boxes for the King's movements, one of the cells is not colored.
 *        Clarification: At this time of execution the possible movements of the piece which defeated the king are colored.
 * @param list <string> values ​​-> List of all the coordinates of the cells valid for the displacements of the king in failure.
 *        The coordinates entered are in the format: 'x-y-bool' where x corresponds to the column of the cell, y corresponds to the row
 *        of the cell and bool corresponds to the attribution of the color true -> blue, false -> pink
 * @return bool -> Determine if the king is checkmate.
 */
bool
MainWindow::IsEchecMat( list<string> values )
{
    bool isEchecMat = true;
    for (string coordonees : values)
    {
        std::vector<std::string> seglist = SplitString( coordonees, '-');

        isEchecMat = this->Echec( std::stoi( seglist.at( 0 ) ) , std::stoi( seglist.at(1) ) ) ;
        if ( !isEchecMat )  break;
    }
    return isEchecMat;
}

/**
 * @brief Displays the pieces eaten
 * @param list <string> PiecesEated -> list of the image paths of the pieces eaten.
 * @param bool white -> Determine if this list is displayed for the white player or the black player.
 */
void
MainWindow::DisplayEatPieces( list<string> PiecesEated, bool white )
{
    QStandardItemModel *monModel = new QStandardItemModel(1, 16) ;
    int i = 0;

    for (string path : PiecesEated)
    {     
        SetPicture( QPixmap ( ConvertToChar( path ) ) , 0 , i , monModel);
        i++;
    }

    if ( white )
        ui->view_PionBlanc->setModel(monModel);
    else
       ui->view_PionNoir->setModel(monModel);
}

/**
 * @brief Adds the movements performed to the movement history and updates the associated visual.
 * @param Piece* laPiece -> The piece that has just performed a movement
 * @param int x -> coordinate of the column of the cell targeted by the movement
 * @param int y -> coordinate of the cell line targeted by the movement
 */
void
MainWindow::AddToHistory( Piece* laPiece,int x, int y )
{
   History.push_back(  std::to_string( laPiece->GetX() ) + ":" + std::to_string( laPiece->GetY() )  + " -> ");
   HistoryPictures.push_back( laPiece->GetPath() );

   HistoryEat.push_back( std::to_string( x )  + ":" +  std::to_string( y ) );

   if ( e.GetPiece( x , y ) != nullptr )
   {
       HistoryPicturesEat.push_back( e.GetPiece( x , y )->GetPath() );
   }
   else
   {
       HistoryPicturesEat.push_back( ":/img/assets/invisible.png" );
   }

   QStandardItemModel* Histo = new QStandardItemModel(this);
   QStandardItemModel* HistoEat = new QStandardItemModel(this);

   int indexLign = 0;
   for (int i = History.size() - 1 ; i > - 1 ; i--)
   {
        QPixmap monImage( ConvertToChar( HistoryPictures.at( i ) ) );
        SetPicture( monImage , indexLign , 0 , Histo);

        QModelIndex index = Histo->index( indexLign,0 , QModelIndex() );
        Histo->setData(index, ConvertToChar( History.at( i ) ) );
        indexLign++;

   }
   indexLign = 0;

   for (int i = HistoryEat.size() - 1 ; i > - 1 ; i--)
   {
          QPixmap monImage2( ConvertToChar( HistoryPicturesEat.at( i ) ) );
          SetPicture( monImage2 , indexLign , 0 , HistoEat);

          QModelIndex index2 = HistoEat->index( indexLign,0 , QModelIndex() );
          HistoEat->setData(index2, ConvertToChar( HistoryEat.at(i) ) );
          indexLign++;
   }

   ui->view_Histo->setModel(Histo);
   ui->view_Histo_2->setModel(HistoEat);
}

/**
 * @brief Convert the text passed as a parameter to type char
 * @param string monText -> Text to convert
 * @return const char * -> Pointer to converted text
 */
const char *
MainWindow::ConvertToChar( string monText )
{
    char const *char_array = monText.c_str();
    return char_array;
}

/**
 * @brief Assigns the color, to the index cell, of the model provided as a parameter
 * @param QColor color -> color to assign
 * @param int i -> coordinate of the column of the cell targeted by the movement
 * @param int j -> coordinate of the cell line targeted by the movement
 * @param QStandardItemModel* model * -> Model on which applied the modifications
 */
void
MainWindow::SetColorBackGround( QColor color ,int i, int j ,QStandardItemModel* model )
{
    QModelIndex index = model->index( i , j , QModelIndex() );
    model->setData( index, QBrush ( color ), Qt::BackgroundRole  );
}

/**
 * @brief Assigns the image, to the index cell, of the model provided as a parameter
 * @param QColor color -> color to assign
 * @param int i -> coordinate of the column of the cell targeted by the movement
 * @param int j -> coordinate of the cell line targeted by the movement
 * @param QStandardItemModel* model * -> Model on which applied the modifications
 */
void
MainWindow::SetPicture ( QPixmap monImage, int i , int j, QStandardItemModel* model )
{
    QIcon* m_icon = new QIcon();
    m_icon->addPixmap(monImage);

    QStandardItem *m_item = new QStandardItem();
    m_item->setIcon(*m_icon);

    model->setItem( i, j , m_item );
}

/**
 * @brief Split a word according to the specified character
 * @param string word -> word to split
 * @param char split -> separation character
 * @return std::vector<std::string> seglist -> split list
 */
std::vector<std::string>
MainWindow::SplitString( string word, char split )
{
    std::stringstream wordStream(word);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(wordStream, segment, split))
    {
       seglist.push_back( segment );
    }
    return seglist;
}

/**
 * @brief Open the game rules documentation
 */
void
MainWindow::ActRegle() { system("start /max https://ecole.apprendre-les-echecs.com/regles-echecs/"); }

/**
 * @brief Switch the app to dark mode
 */
void
MainWindow::ActDarkMode()
{
    this->setStyleSheet("background-color: rgb(88, 88, 88);");
    ui->tableViewEchiquier->setStyleSheet("background-color: rgb(212, 212, 212);");
    ui->groupBox->setStyleSheet("background-color: rgb(212, 212, 212);");
    ui->view_PionBlanc->setStyleSheet("background-color: rgb(212, 212, 212);");
    ui->view_PionNoir->setStyleSheet("background-color: rgb(212, 212, 212);");
}

/**
 * @brief Switch the application to clear mode
 */
void
MainWindow::ActLightMode()
{
    this->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->tableViewEchiquier->setStyleSheet("");
    ui->groupBox->setStyleSheet("");
    ui->view_PionBlanc->setStyleSheet("");
    ui->view_PionNoir->setStyleSheet("");
}

/**
 * @brief Open the application documentation
 */
void
MainWindow::ActDocumentation() { }

void
MainWindow::ShowTime()
{
    QTime time = QTime(Hours,minutes,seconds) ;
    QString time_text = time.toString("hh:mm:ss");
    ui->lcdTimer->display( (time_text) );
}

void
MainWindow::SetTimer()
{
   timer->setInterval(1000);
   seconds++;

   if ( seconds == 60 )
   {
       seconds = 0;
       minutes++;
   }
   if ( minutes == 60 )
   {
       Hours++;
       minutes = 0;
   }
   ShowTime();
}
/**
 * @brief MainWindow::EndGameDisplay
 */
void
MainWindow::EndGameDisplay()
{
    QMessageBox::StandardButton endDisplay = QMessageBox::information( this, "Chess",
                                                                       tr("END GAME \n"),
                                                                       QMessageBox::Ok,
                                                                       QMessageBox::Ok);
    if (endDisplay != QMessageBox::Ok) {
        // ok boomer
    } else {
        this->close();
        ui->tableViewEchiquier->setEnabled(false);
    }
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void
MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Chess",
                                                                tr("Are you sure ? \n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();

    } else {
        event->accept();
    }
}


