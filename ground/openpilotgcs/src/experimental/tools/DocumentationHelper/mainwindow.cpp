/**
******************************************************************************
*
* @file       mainwindow.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
* @brief
* @see        The GNU Public License (GPL) Version 3
* @defgroup   DocumentationHelper
* @{
*
*****************************************************************************/
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    license<<"/**";
    license<<"******************************************************************************";
    license<<"*";
    license<<"* @file       %file";
    license<<"* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.";
    license<<"* @brief      ";
    license<<"* @see        The GNU Public License (GPL) Version 3";
    license<<"* @defgroup   %defgroup";
    license<<"* @{";
    license<<"* ";
    license<<"*****************************************************************************/";
    license<<"/* ";
    license<<"* This program is free software; you can redistribute it and/or modify ";
    license<<"* it under the terms of the GNU General Public License as published by ";
    license<<"* the Free Software Foundation; either version 3 of the License, or ";
    license<<"* (at your option) any later version.";
    license<<"* ";
    license<<"* This program is distributed in the hope that it will be useful, but ";
    license<<"* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ";
    license<<"* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License ";
    license<<"* for more details.";
    license<<"* ";
    license<<"* You should have received a copy of the GNU General Public License along ";
    license<<"* with this program; if not, write to the Free Software Foundation, Inc., ";
    license<<"* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA";
    license<<"*/";

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_cpathBT_clicked()
{
    wpath=QFileDialog::getExistingDirectory(this,"Choose Work Path");
    ui->cpathL->setText("Current Path:"+wpath);
}

void MainWindow::on_goBT_clicked()
{
    QDir myDir(wpath);
    QStringList filter;
    filter<<"*.cpp"<<"*.h";
    QStringList list = myDir.entryList (filter);

    foreach(QString str,list)
    {
        bool go=true;
        if(ui->confirmCB->isChecked())
            if(QMessageBox::question(this,"Process File?:",str,QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)go=false;
        if(go){
            ui->cfileL->setText("Current File:"+str);
            QFile file(wpath+QDir::separator()+str);
            if (!file.open(QIODevice::ReadWrite)) {
                ui->output->append("Cannot open file "+str+" for writing");
            }
            else
            {
                ui->output->append("Processing file "+str);
                QTextStream out(&file);
                QStringList filestr;
                out.seek(0);
                while(!out.atEnd()) filestr<<out.readLine();
                out.seek(0);
                if(ui->licenseCB->isChecked())
                {

                    bool haslicense=false;
                    foreach(QString str,filestr)
                    {
                        if(str.contains("The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010"))
                        {
                            haslicense=true;
                            ui->output->append("File Already has License Info");
                        }
                    }

                    if(!haslicense)
                    {
                        bool process=true;
                        if(ui->confirmCB->isChecked())
                            if(QMessageBox::question(this,"Add license Info?:",str,QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) process=false;
                        if(process)
                        {
                            ui->output->append("Added License info to file");
                            out.seek(0);
                            foreach(QString line,license)
                            {
                                line.replace("%file",str);
                                line.replace("%defgroup",ui->defgroup->text());
                                out<<line<<"\r\n";
                            }
                            foreach(QString str,filestr) out<<str+"\r\n";
                        }
                    }
                    out.flush();
                }
                if(ui->nameCB->isChecked())
                {
                    filestr.clear();
                    out.seek(0);
                    while(!out.atEnd()) filestr<<out.readLine();
                    out.reset();
                    bool process=true;
                    foreach(QString s,filestr)
                    {
                        if (s.contains("namespace"))
                        {
                            process=false;
                            ui->output->append("File Already has Namespace");
                        }
                    }

                    if(ui->confirmCB->isChecked() && process)
                        if(QMessageBox::question(this,"Create Namespace?:",str,QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) process=false;
                    if(process)
                    {
                        ui->output->append("Added NameSpace to file");
                        out.seek(0);
                        bool initdone=false;
                        for(int x=0;x<filestr.count();++x)
                        {
                            QString line=filestr.at(x);
                            if(!initdone)
                            {
                                if(!(line.trimmed().startsWith("#")||line.trimmed().startsWith("/")||line.trimmed().startsWith("*")||line.trimmed().isEmpty()))
                                {
                                    filestr.insert(x,"namespace "+ui->namespa->text()+" {");
                                    filestr.insert(x," ");
                                    initdone=true;
                                }
                            }
                            else
                            {
                                if((QString)str.split(".").at(1)=="cpp")
                                {
                                    filestr.append("}");
                                    break;
                                }
                                else
                                {
                                    for(int y=filestr.count()-1;y>1;--y)
                                    {
                                        QString s=filestr.at(y);
                                        if(s.contains("#endif"))
                                        {
                                            filestr.insert(y,"}");
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        foreach(QString str,filestr) out<<str+"\r\n";
                        out.flush();
                    }
                }
                if(ui->bockifCB->isChecked())
                {
                    QString genif;
                    ui->output->append("Creating block ifs");
                    filestr.clear();
                    out.seek(0);
                    while(!out.atEnd()) filestr<<out.readLine();
                    out.seek(0);
                    for(int x=1;x<filestr.count()-1;++x)
                    {
                        QString before=filestr.at(x-1);
                        QString actual=filestr.at(x);
                        QString after=filestr.at(x+1);
                        if(actual.trimmed().startsWith("qDebug")&&!before.contains("#ifdef")&&!before.contains("qDebug"))
                        {
                            filestr.insert(x,"#ifdef DEBUG_"+(QString)str.split(".").at(0).toUpper());
                            ++x;
                            genif="#define DEBUG_"+(QString)str.split(".").at(0).toUpper();
                        }
                        if(actual.trimmed().startsWith("qDebug")&&!after.contains("qDebug")&&!after.contains("#endif"))
                        {
                            filestr.insert(x+1,"#endif //DEBUG_"+(QString)str.split(".").at(0).toUpper());
                        }

                    }
                    if(!genif.isEmpty())ui->textEdit->append(genif);
                    foreach(QString str,filestr) out<<str+"\r\n";
                    out.flush();

                }
                file.close();
            }

        }
    }
}
