/**
 * @authors Yoan Laurain ; Hugo Carricart ; Nathan Lesourd
 * @brief Source Code de MainWindow
 * @file mainwindow.cpp
 * @date 26/10/2021
 * @version 0.5
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Echiquier.h"
#include "Piece.h"
#include "Joueur.h"
#include <iostream>
#include <assert.h>
#include "mainwindow.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    assert( jn.placerPieces( e ) );
    assert( jb.placerPieces( e ) );

    RoiBlanc = e.getPiece( 5 , 1 );
    RoiNoir  = e.getPiece( 5 , 8 );

    QPixmap monImage(":/img_blanc/assets/blanc/pion.png");
    ui->TourLabel->setAlignment(Qt::AlignCenter);
    ui->TourLabel->setPixmap(monImage);

    this->RefreshMatrice(this);
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() { delete ui; }

/**
 * @brief MainWindow::RefreshMatrice
 * @param parent
 */
void
MainWindow::RefreshMatrice(QWidget *parent)
{
    model = new QStandardItemModel(8,8,parent) ;

    for ( int i = 0 ; i < 8 ; i++ )
        for ( int j = 0 ; j < 8 ; j++ )
        {
            if ( e.getPiece( j + 1 , i + 1 ) != nullptr )
            {
               string s = e.getPiece( j + 1 , i + 1 )->path();
               int n    = s.length();
               char char_array[ n + 1 ];
               strcpy( char_array, s.c_str() );
               QPixmap monImage( char_array );

               QIcon* m_icon = new QIcon();
               m_icon->addPixmap(monImage);

               QStandardItem *m_item = new QStandardItem();
               m_item->setIcon(*m_icon);

               model->setItem( i, j , m_item );
            }

            if (  e.getPiece(j+1, i+1) != nullptr && e.getPiece( j + 1 , i + 1 )->isEchec() )
            {
                QModelIndex index = model->index( i , j , QModelIndex() );
                model->setData( index, QBrush ( QColor ( "red" )  ), Qt::BackgroundRole  );
            }
            if ( i % 2 == j % 2 )
            {
                QModelIndex index = model->index( i , j , QModelIndex() );
                QColor color(103,159,90, 150); // vert mousse
                model->setData( index, QBrush ( color ), Qt::BackgroundRole  );
            }
            else
            {
                QModelIndex index = model->index( i , j , QModelIndex() );
                QColor color(225, 206, 154, 150);// autre
                model->setData( index, QBrush ( color ), Qt::BackgroundRole  );
            }
    }
    if ( WhitePlay == true )
    {
        QPixmap monImage( ":/img_blanc/assets/blanc/pion.png" );
        ui->TourLabel->setAlignment( Qt::AlignCenter );
        ui->TourLabel->setPixmap( monImage );
    }
    else
    {
        QPixmap monImage( ":/img_noir/assets/noir/pion.png" );
        ui->TourLabel->setAlignment( Qt::AlignCenter );
        ui->TourLabel->setPixmap( monImage );
    }
    ui->tableViewEchiquier->setModel( model );
    ui->tableViewEchiquier->setIconSize( QSize( 90 , 90 ) );
}

/**
 * @brief MainWindow::on_tableViewEchiquier_clicked
 * @param index
 */
void
MainWindow::on_tableViewEchiquier_clicked(const QModelIndex &index)
{
    QVariant selectedCell      = model->data( index, Qt::BackgroundRole );
    QColor colorOfSelectedCell = selectedCell.value<QColor>();

    if ( pieceEnCours != nullptr && pieceEnCours->isWhite() == WhitePlay && colorOfSelectedCell.value() == 255 )
    {

        e.deplacer( pieceEnCours , index.column()+1  , index.row()+1 );

        if ( index.column()+1 == RoiBlanc->x() && index.row()+1 ==  RoiBlanc->y() )  RoiBlanc = nullptr;
        if ( index.column()+1 == RoiNoir->x() && index.row()+1 ==  RoiNoir->y() )  RoiNoir = nullptr;

        if (RoiNoir == nullptr || RoiBlanc == nullptr)
        {
            cout << "fin de game" << endl;
        }
        else
        {
            if ( WhitePlay )
            {
               this->setColor( pieceEnCours->AfficheMouvementValide( e , WhitePlay ) );

               if ( this->Echec( RoiNoir->x() , RoiNoir->y() ) )
               {
                   bool isEchecMat = this->IsEchecMat( pieceEnCours->MouvementPossibleRoi( e , RoiNoir->x() , RoiNoir->y() ) );

                   if ( isEchecMat )
                   {
                       cout << "Echec et mat" << endl;
                       //Fin de partie
                   }
                   RoiNoir->setIsEchec();
               }
            }
            else
            {
                this->setColor( pieceEnCours->AfficheMouvementValide(e,WhitePlay));

                if ( this->Echec( RoiBlanc->x() , RoiBlanc->y() ) )
                {
                    bool isEchecMat = this->IsEchecMat( pieceEnCours->MouvementPossibleRoi( e , RoiBlanc->x() , RoiBlanc->y() ) );

                    if ( isEchecMat )
                    {
                        cout << "Echec et mat" << endl;
                        //Fin de partie
                    }

                    RoiBlanc->setIsEchec();
                }
            }
        }

        WhitePlay = !WhitePlay;
        this->RefreshMatrice(this);

    }
    else
    {
        pieceEnCours = e.getPiece(  index.column()+1  , index.row()+1 );
        this->RefreshMatrice(this);
        if ( pieceEnCours != nullptr ) this->setColor( pieceEnCours->AfficheMouvementValide(e,WhitePlay) );
    }  
}

/**
 * @brief MainWindow::setColor
 * @param values
 */
void
MainWindow::setColor(list<string>values)
{
   for (string coordonees : values)
   {
       std::stringstream test(coordonees);
       std::string segment;
       std::vector<std::string> seglist;

       while(std::getline(test, segment, '-'))
       {
          seglist.push_back( segment );
       }

       try {

           QColor color = ( seglist.at(2) == "true" ? "red" : "blue"  );
           QModelIndex index = model->index( std::stoi( seglist.at(1) ) , std::stoi( seglist.at(0) ) ,QModelIndex());
           model->setData(index, QColor( color ), Qt::BackgroundRole  );
       }  catch (...) {}
   }

   ui->tableViewEchiquier->setModel(model);
}

/**
 * @brief MainWindow::Echec
 * @param x
 * @param y
 * @return
 */
bool
MainWindow::Echec ( int x , int y)
{
    QModelIndex index = model->index( y - 1,x - 1 , QModelIndex() );
    QVariant selectedCell      = model->data( index, Qt::BackgroundRole );
    QColor colorOfSelectedCell = selectedCell.value<QColor>();

    return ( colorOfSelectedCell.value() == 255 ? true : false );
}

bool
MainWindow::IsEchecMat( list<string> values)
{
    bool isEchecMat = true;
    for (string coordonees : values)
    {
        std::stringstream test(coordonees);
        std::string segment;
        std::vector<std::string> seglist;

        while(std::getline(test, segment, '-'))
        {
           seglist.push_back( segment );
        }

        isEchecMat = this->Echec( std::stoi( seglist.at( 0 ) ) , std::stoi( seglist.at(1) ) ) ;
        if ( !isEchecMat )  break;
    }
    return isEchecMat;
}
