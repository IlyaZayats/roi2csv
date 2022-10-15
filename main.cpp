#include <iostream>

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmimage/diregist.h" /*to support color image*/
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmsr/dsrdoc.h"
#include "dcmtk/dcmsr/dsrdoctr.h"
#include "dcmtk/dcmsr/dsrdocst.h"
#include "dcmtk/dcmsr/dsrcodvl.h"
#include "dcmtk/dcmsr/dsrrefin.h"
#include "dcmtk/dcmdata/dcjson.h"

#include <filesystem>
#include <vector>
#include <typeinfo>

#include "QFile"
#include "QJsonObject"
#include "QJsonDocument"
#include "QJsonArray"
#include <QCoreApplication>
#include "QString"

using namespace std;
namespace stdfs = std::filesystem;

struct output{
    QString study;
    QString file;
    QString roi;
};

DcmJsonFormatPretty fmt(OFTrue);

void parseToJson(string file_name, DcmFileFormat dcm){
    DcmDataset *dset = dcm.getDataset();
    //fmt.setJsonExtensionEnabled(OFFalse);
    //fmt.setJsonNumStringPolicy(DcmJsonFormat::NSP_auto);
    ofstream stream;
    stream.open(file_name);
    dset->writeJsonExt(stream, fmt, OFTrue, OFTrue);
    //dcm.writeJson(stream, fmt);
    stream.close();
}

QString getStudyName(QJsonObject json){
    QString result = "";
    QJsonValue val1 = json.value("00100010");
    if(val1.isObject()){
        QJsonObject obj1 = val1.toObject();
        QJsonArray val2 = obj1["Value"].toArray();
        //qDebug() << val2;
        if(!val2.isEmpty()) {
            QJsonObject obj2 = val2[0].toObject();
            //qDebug() << obj2;
            QJsonValue val3 = obj2.value("Alphabetic");
            //qDebug() << val3.toString();
            result = val3.toString();
        } else {
            cerr << "err2";
        }
    } else {
        cerr << "err1";
    }
    return result;
}

QString getUID(QJsonObject json){
    QString result = "";
    QJsonValue val;
    QJsonObject obj = json;
    QJsonArray arr;
    QString keys [] = {"0040A730", "00081199", "00081155"};
    for(int i=0; i<3; i++){
        val = obj.value(keys[i]);
        obj = val.toObject();
        arr = obj.value("Value").toArray();
        obj = arr[0].toObject();
    }
    result = arr[0].toString();
    return result;
}

QString getFile(QJsonObject json, QString file_uid){
    QString result = "";
    QJsonValue value = json.value("00041220");
    QJsonObject obj = value.toObject();
    QJsonArray arr = obj.value("Value").toArray();
    //qDebug() << arr.size();

    for(int i=0; i<arr.size(); i++){
        QJsonObject arr_obj = arr[i].toObject();
        QJsonValue uid = arr_obj.value("00041511");
        //qDebug() << uid << Qt::endl;
        QJsonObject uid_obj = uid.toObject();
        QJsonArray uid_value = uid_obj.value("Value").toArray();
        //qDebug() << uid_value[0] << Qt::endl;
        if(uid_value[0] == file_uid){
            QJsonValue file = arr_obj.value("00041500");
            QJsonObject file_obj = file.toObject();
            QJsonArray file_arr = file_obj.value("Value").toArray();
            result = file_arr[0].toString();
            //qDebug() << result << Qt::endl;
            break;
        }
        //qDebug() << uid_value[0] << Qt::endl;
    }

    return result;
}

QString getROI(QJsonObject json){
    QString result = "";
    QJsonValue val = json.value("00420011");
    QJsonObject obj = val.toObject();
    val = obj.value("InlineBinary");
    result = val.toString();
    return result;
}

QJsonObject getJson(QString file_name){
    QString val;
    QFile input;
    input.setFileName(file_name);
    input.open(QIODevice::ReadOnly | QIODevice::Text);
    val = input.readAll();
    input.close();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8(), &err);
    if(err.error != 0) {
        qDebug() << "Error: " << err.errorString() << err.offset << err.error;
    }
    return doc.object();
}

void outputFileNames(){

}

int main() {
    QFile csv("result.csv");
    if(!csv.open(QIODevice::ReadOnly | QIODevice::Text)){
        if(csv.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream out(&csv);
            out << "Study,File,ROI" << "\n";
        }
    }
    csv.close();
//    ofstream csv;
//    csv.open("result.csv", ios::app);
//    csv.close();
//    fstream csv_f("result.csv");
//    if(csv_f.peek() == EOF){
//        csv_f.close();
//        csv.open("result.csv");
//        csv << "Study,File,ROI" << endl;
//    }

    stdfs::path main_path = "/Users/ilya/Documents/datasets/CTAG-2/";
    const stdfs::directory_iterator main_end{};
    for(stdfs::directory_iterator iterator{ main_path }; iterator != main_end; ++iterator) {
        //for macOS
        if(iterator->path()!="/Users/ilya/Documents/datasets/CTAG-2/.DS_Store") {
        //
            vector<output> data;
            stdfs::path path = iterator->path();
            //cout << iterator->path();

            const stdfs::directory_iterator end{};
            for (stdfs::directory_iterator iter{path}; iter != end; ++iter) {
                DcmFileFormat file;
                //qDebug() << iter->path() << Qt::endl;

                //if statement for macOS
                if (iter->path() != "/Users/ilya/Documents/search/.DS_Store") {
                    if (file.loadFile(iter->path().string().c_str()).good()) {
                        //cout << "good" << endl;
                        DcmMetaInfo *info = file.getMetaInfo();
                        int size = info->calcElementLength(EXS_Unknown, EET_ExplicitLength);
                        //cout << "size: " << size << endl;
                        //322 - DICOMDIR
                        if (size == 322) {
                            parseToJson("dir.json", file);
                        }
                        //334 - DICOMSR
                        if (size == 334) {
                            parseToJson("sr.json", file);
                            QJsonObject json = getJson("sr.json");
                            output out;
                            out.study = getStudyName(json);
                            out.file = getUID(json);
                            out.roi = getROI(json);

//                            qDebug() << out.study;
//                            qDebug() << out.file;
//                            qDebug() << out.roi;

                            data.push_back(out);
                        }

                    } else {
                        cerr << "bruh" << endl;
                    }
                }
            }

            QJsonObject json = getJson("dir.json");
            for (vector<output>::iterator it = data.begin(); it != data.end();) {
                QString file_name = it->file, file_uid = it->file;
                file_name = getFile(json, file_name);
                if (file_name == "") {
                    qDebug() << "File with uid =" << file_uid << "doesnt exist in DICOMDIR";
                    data.erase(it);
                } else {
                    it->file = file_name;
                    qDebug() << "File with uid =" << file_uid << "found in DICOMDIR and its name is" << file_name;
                    csv.open(QIODevice::WriteOnly | QIODevice::Append);
                    QTextStream out(&csv);
                    QByteArray arr = QByteArray::fromBase64(it->roi.toUtf8());
                    QString coords = "";
                    for (int i = arr.indexOf("{{") + 1; i < arr.indexOf("}}") + 1; i++) {
                        coords.push_back(arr[i].operator char());
                    }
                    if(!coords.isEmpty()) {
                        out << it->study << "," << it->file << ",(" << coords << ")\n";
                    }
                    csv.close();
                    ++it;
                }
            }
        }


    }
    return 0;
}
