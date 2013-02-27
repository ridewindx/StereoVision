#include "imageIO.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <QMessageBox>

//#include <pcl/io/pcd_io.h>
//#include <pcl/point_types.h>
#include <limits>

using namespace std;

bool writeBytesToFile(const string fileName,const unsigned char *buffer,const int size)
{
    ofstream outFile;
    outFile.open(fileName.c_str(),ios::out|ios::binary);
    if(!outFile.is_open()){
        cerr<<"Failed to open "<<fileName<<" for writing"<<endl;
        return false;
    }
    outFile.write((const char *)buffer,size);
    outFile.close();

    return true;
}

bool readBytesFromFile(const string fileName,unsigned char *buffer,const int size)
{
    ifstream inFile;
    inFile.open(fileName.c_str(),ios::in|ios::binary);
    if(!inFile.is_open()){
        cerr<<"Failed to open "<<fileName<<" for reading"<<endl;
        return false;
    }
    inFile.read((char *)buffer,size);
    inFile.close();

    return true;
}

bool writeSingleImage3D(const std::string filename,IMAGE3D *image,const int &width,const int &height)
{
    return writeMultiImage3D(filename,image,width,height,1);
}

bool writeMultiImage3D(const std::string filename,IMAGE3D *image,const int &width,const int &height,int num)
{
    const int size=width*height*num;
    std::auto_ptr<double> data(new double[size*3]);
    std::auto_ptr<unsigned char> texture(new unsigned char[size*3]);
    std::auto_ptr<unsigned char> mask(new unsigned char[size]);
    if(!data.get() || !texture.get() || !mask.get()){
        std::cerr<<"Allocation failed while writing Image3D data!"<<std::endl;
        return false;
    }

    for(int i=0;i<size;i++){
        for(int j=0;j<3;j++){
            data.get()[i*3+j]=image[i].coor[j];
            texture.get()[i*3+j]=(unsigned char)(image[i].text[j]*255);
        }
        mask.get()[i]=image[i].mask?1:0;
        /*
        if(image[i].mask){
            std::cout<<image[i].coor[0]<<"  "<<image[i].coor[1]<<"  "<<image[i].coor[2]<<"  "<<std::endl;
        }
        */
    }

    std::ofstream out;
    out.open(filename.c_str(),std::ios::out|std::ios::binary);
    if(!out.is_open()){
        std::cerr<<"Failed to open "<<filename<<" for writing"<<std::endl;
        return false;
    }

    out.write((const char*)&width,sizeof(width));
    out.write((const char*)&height,sizeof(height));

    out.write((const char*)data.get(),size*3*sizeof(data.get()[0]));
    out.write((const char*)texture.get(),size*3);
    out.write((const char*)mask.get(),size);
    out.close();

    return true;
}

bool readSingleImage3D(const string fileName,IMAGE3D *image)
{
    return readMultiImage3D(fileName,image,1);
}

bool readMultiImage3D(const string fileName,IMAGE3D *image,int num)
{
    int width,height,size;
    ifstream inFile(fileName.c_str(),ios::in|ios::binary);
    if(!inFile.is_open()){
        cerr<<"Failed to open "<<fileName<<" for reading"<<endl;
        return false;
    }

    inFile.read((char *)&width,sizeof(width));
    inFile.read((char *)&height,sizeof(height));

    size=width*height*num;

    std::auto_ptr<double> data(new double[size*3]);
    std::auto_ptr<unsigned char> texture(new unsigned char[size*3]);
    std::auto_ptr<unsigned char> mask(new unsigned char[size]);
    if(!data.get() || !texture.get() || !mask.get()){
            std::cerr<<"Allocation failed while reading Image3D data!"<<std::endl;
            return false;
    }

    inFile.read((char *)data.get(),size*3*sizeof(data.get()[0]));
    inFile.read((char *)texture.get(),size*3);
    inFile.read((char *)mask.get(),size);
    inFile.close();

    for(int i=0;i<size;i++){
        for(int j=0;j<3;j++){
            image[i].coor[j]=data.get()[i*3+j];
            image[i].text[j]=texture.get()[i*3+j]/255.0;
        }
        image[i].mask=static_cast<bool>(mask.get()[i]);
        /*
        if(image[i].mask){
            std::cout<<image[i].coor[0]<<"  "<<image[i].coor[1]<<"  "<<image[i].coor[2]<<"  "<<std::endl;
        }
        */
    }

    //计算单位法向量
    Point p0,p1,p2,pNormal;
    for(int i=0;i<num;i++){
        for(int j=0;j<height-1;j++){
            for(int k=0;k<width-1;k++){
                int n=i*width*height+j*width+k; //像素索引，不计算最右列和最下行
                /*
                p0.----->.p1
                  |
                  |
                  |
                  v
                P2.
                */
                p0=image[n].coor;
                p1=image[n+1].coor;
                p2=image[n+width].coor;

                if(image[n].mask && image[n+1].mask && image[n+width].mask){ //如果数据有效，则计算
                    pNormal=(p0-p2)^(p0-p1); //计算向量积，法向量垂直纸面向外
                    image[n].norm=pNormal/pNormal.norm(); //单位化
                }
            }
        }
    }

    return true;
}

//bool readFromPCD(IMAGE3D *image, int width, int height, const string fileNameCoorNorm)
//{
//    int size=width*height;

//    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal> ());
//    pcl::io::loadPCDFile<pcl::PointNormal>(fileNameCoorNorm, *cloud);

//    for(int i=0,k=0;i<size;i++){
//        if(image[i].mask && k<cloud->width){
//            for(int j=0;j<3;j++){
//                image[i].text[j]=0;
//                switch(j){
//                case 0:
//                    image[i].coor[j]=cloud->points[k].x;
//                    image[i].norm[j]=cloud->points[k].normal_x;
//                    break;
//                case 1:
//                    image[i].coor[j]=cloud->points[k].y;
//                    image[i].norm[j]=cloud->points[k].normal_y;
//                    break;
//                case 2:
//                    image[i].coor[j]=cloud->points[k].z;
//                    image[i].norm[j]=cloud->points[k].normal_z;
//                    break;
//                }
//            }
//            k++;
//        }
//    }

//    //计算单位法向量
//    Point p0,p1,p2,pNormal;
//        for(int j=0;j<height-1;j++){
//            for(int k=0;k<width-1;k++){
//                int n=j*width+k; //像素索引，不计算最右列和最下行
//                /*
//                p0.----->.p1
//                  |
//                  |
//                  |
//                  v
//                P2.
//                */
//                p0=image[n].coor;
//                p1=image[n+1].coor;
//                p2=image[n+width].coor;

//                if(image[n].mask && image[n+1].mask && image[n+width].mask){ //如果数据有效，则计算
//                    pNormal=(p0-p2)^(p0-p1); //计算向量积，法向量垂直纸面向外
//                    image[n].norm=pNormal/pNormal.norm(); //单位化
//                }
//            }
//        }

//    return true;
//}

bool writeImage3DCoordinates(const std::string fileName,IMAGE3D *image,int width,int height)
{
    ofstream outFile(fileName.c_str());
    if(!outFile.is_open()){
        cerr<<"Failed to open "<<fileName<<" for writing"<<endl;
        return false;
    }

    for(int i=0;i<width;i++){
        for(int j=0;j<height;j++){
            int idx=i+width*j;
            if(image[idx].mask){
                //outFile<<i<<" "<<j<<" "<<image[idx].coor[0]<<" "<<image[idx].coor[1]<<" "<<image[idx].coor[2]<<endl;
                outFile<<image[idx].coor[0]<<" "<<image[idx].coor[1]<<" "<<image[idx].coor[2]<<endl;
            }
            else{
                //outFile<<i<<" "<<j<<" "<<0<<" "<<0<<" "<<0<<endl;
            }
        }
    }

    outFile.close();

    return true;
}

//bool writeToPCD(const std::string fileName,IMAGE3D *image,int width,int height)
//{
//    pcl::PointCloud<pcl::PointXYZ> cloud;

//    // Fill in the cloud data
//    cloud.width    = width;
//    cloud.height   = height;
//    cloud.is_dense = true;
//    cloud.points.resize (cloud.width * cloud.height);

//    float nan=std::numeric_limits<float>::quiet_NaN();

//    for(int i=0;i<height;i++){
//        for(int j=0;j<width;j++){
//            int idx=j+width*i;
//            if(image[idx].mask && j>=int(250+(5*rand()/(RAND_MAX+1.0))) && i>int(0+(10*rand()/(RAND_MAX+1.0))) ){ //250,700
//            //if(image[idx].mask && j<=(int)(700+(5*rand()/(RAND_MAX+1.0))) && i<int(768-(40*rand()/(RAND_MAX+1.0))) ){
//                cloud.points[idx].x=float(image[idx].coor.x);
//                cloud.points[idx].y=float(image[idx].coor.y);
//                cloud.points[idx].z=float(image[idx].coor.z);


//                cloud.points[idx].x+=0.1 * rand () / (RAND_MAX + 1.0);
//                cloud.points[idx].y+=0.5 * rand () / (RAND_MAX + 1.0);
//                cloud.points[idx].z+=0.1 * rand () / (RAND_MAX + 1.0);


//                /*
//                cloud.points[idx].x+=0.05 * rand () / (RAND_MAX + 1.0);
//                cloud.points[idx].y+=1 * rand () / (RAND_MAX + 1.0);
//                cloud.points[idx].z+=0.05 * rand () / (RAND_MAX + 1.0);
//                */
//                /*
//                float y,z,pi=3.1415926;
//                y=cloud.points[idx].y*cos(-pi/12)-cloud.points[idx].z*sin(-pi/12);
//                z=cloud.points[idx].y*sin(-pi/12)+cloud.points[idx].z*cos(-pi/12);
//                cloud.points[idx].y=y;
//                cloud.points[idx].z=z;
//                */
//            }
//            else{
//                cloud.points[idx].x=nan;
//                cloud.points[idx].y=nan;
//                cloud.points[idx].z=nan;
//            }
//        }
//    }

//    if(pcl::io::savePCDFileBinary (fileName, cloud) != 0){
//        return false;
//    }

//    return true;
//}
