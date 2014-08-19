#ifndef FILE_TREE_HPP_INCLUDE
#define FILE_TREE_HPP_INCLUDE

#include <stdio.h>  
  
#include <iostream>  
#include <sstream>  
#include <string>  
#include <locale>  
#include <direct.h> //创建目录


#include<fstream>
#include<iomanip>
#include<io.h>
//#include<time.h>
//#include<sstream>
//#include <windows.h> 
  
#include "boost/property_tree/ptree.hpp"  
#include "boost/property_tree/json_parser.hpp"  
#include "boost/property_tree/ini_parser.hpp"  
#include "boost/property_tree/xml_parser.hpp"  



 
 using namespace std;
 using namespace boost::property_tree;

/*
   将str_root中的Oldestr替换为Newstr
*/
string ReplaceString(string str_root,string Oldstr,string Newstr){

	int pos = str_root.find(Oldstr);
	if(pos >= int(str_root.length())){
		return str_root;
	}

	return str_root.replace(pos,Oldstr.length(),Newstr);
}


/*
   从全路径截取出box名
   例如:想从全路径"box1/folder1/file.txt"得到"box1"
        则GetBoxNameFromFullPathInBox("box1/folder1/file.txt");
   ============================================================
   in:
	FullPathInBox 要截取的全路径
   ------------------------------------------------------------
   out:(none)
   ------------------------------------------------------------
   return:
	box件名
*/
string GetBoxNameFromFullPathInBox(string FullPathInBox){

	
	string BoxName;
	if(FullPathInBox.find('/') == 0 || FullPathInBox.find('/') >= FullPathInBox.length()){
		return FullPathInBox;
	}else{
		BoxName = FullPathInBox.substr(0,FullPathInBox.find('/'));
	}

	return BoxName;
}

/*
   从全路径截取出文件名
   例如:想从全路径"folder1\folder2\file.txt"得到"file.txt"
        则GetFileNameFromFullPath("folder1\folder2\file.txt");
   ============================================================
   in:
	FullPath 要截取的全路径
   ------------------------------------------------------------
   out:(none)
   ------------------------------------------------------------
   return:
	文件名
*/
string GetFileNameFromFullPath(string FullPath,string dot = "\\"){


	string FileName;
	if(FullPath.find_last_of(dot) >= FullPath.length()-1){
		return FullPath;
	}else{
		FileName = FullPath.substr(FullPath.find_last_of(dot)+1);
	}


	return FileName;
}

namespace filetree{
int AddFileToFloderNode(ptree& pt_Folder,string FileName,string OtherFullPathInBox);

 ostream& operator<<( ostream& os, const pair<int, const ptree&>& rNode )
{
  int iNext = rNode.first + 2;
  const ptree& rPT = rNode.second;


	os << " Value: [" << rPT.data() << "]\n";
  

  for( auto it = rPT.begin(); it != rPT.end(); ++ it )
  {
    os.width( iNext );
    os << " ";
    os << "Name: [" << it->first << "]";
    os << pair<int, const ptree&>( iNext, it->second );
  }
  return os;
}

void printTree(const pair<int, const ptree&>& rNode )
{  
	if(rNode.second.empty() == true){//不再含有分支
		
	}else{
		for(auto it = rNode.second.begin();it != rNode.second.end();++it){
			if(it->first == "<xmlattr>"){
				for(auto ittemp = it->second.begin();ittemp!= it->second.end();++ittemp){
					cout<<"  "<<ittemp->first<<"="<<ittemp->second.data()<<" ";
				}
				cout<<endl;
			}else{
				cout<<endl;
				cout<<rNode.first<<":"<<it->first;
				printTree(pair<int, const ptree&> (rNode.first+1,it->second ));
			}
			
		}
		cout<<endl;
	}
}  

/*
  新建一个box,其名字为boxname
  in:
  pt root的地址
  boxname 新box的名字
  out:(none)
  return:
  -1 创建失败
  -2 已存在boxname
  1 创建成功
*/
int AddNewBox(ptree& pt,string boxname){
	//检查是否含有root
	ptree NodeBox;

	try{

		NodeBox = pt.get_child("root.boxs");
		//检查boxname是否重复
		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name","") == boxname){
				return -2;
			}
		}
	}
	catch(...){
		cout<<"没找到root.boxs"<<endl;
	}

	pt.put("root.numbox",pt.get<int>("root.numbox",0)+1);
	
	
	NodeBox.clear();
	NodeBox.put("name",boxname);
	NodeBox.put("numfile",0);
	NodeBox.put("numfolder",0);
	
	pt.add_child("root.boxs.box",NodeBox);


	return 1;
	
}

/*
  判断在文件节点下是否存otherpath
  in:
  pt 判断的节点，文件节点
  otherpath 判断otherpath是否存在
  CreatOtherPathInNoexist 如果otherpath不存在是否创建

  out:(none)

  return:

  true 存在
  false 不存在

*/
bool SameOtherPathInfileNode(ptree& pt,string otherpath,bool CreatOtherPathInNoexist = false){

	if(pt.empty() == true){
		return false;
	}else{
		try{
			for(auto it = pt.get_child("otherpaths").begin();it != pt.get_child("otherpaths").end();++it){
				if(it->second.get<string>("otherpath","") == otherpath){
					return true;
				}
			}
		}
		catch(...){
		}

		if(CreatOtherPathInNoexist){
			pt.put("otherpaths.otherpath",otherpath);
			return true;
		}else{
			return false;
		}
	}

}

/*
  判断在同一节点下是否存name的文件（夹）
  in:
  pt 判断的节点，文件夹节点("folders")或文件节点("files")
  name 要判断的文件(夹)的名
  otherpath 判断otherpath是否存在
  CreatOtherPathInNoexist 如果otherpath不存在是否创建

  out:(none)

  return:
  2 存在，且otherpath存在
  1 存在，otherpath不存在
  -1 不存在

*/
int SameNameInOneNode(ptree& pt,string name,string otherpath = "",bool CreatOtherPathInNoexist = false){

	if(pt.empty() == true){
		return false;
	}else{
		for(auto it = pt.begin();it != pt.end();++it){
			if(it->second.get<string>("name","") == name){
				if(otherpath != ""){
					if(SameOtherPathInfileNode(it->second,otherpath,CreatOtherPathInNoexist)){
						return 2;
					}else{
						return 1;
					}
				}else{
					return 1;
				}
			}
		}
		return -1;
	}

}

/*
  直接将文件添加至指定Box
  in:
   pt_root root节点地址
   FileName 要添加文件的文件名
   BoxName 指定Box的名字
   creatifnotfound 如果指定不存在是否创建，默认为不创建
  out:（none)
  return:
   1为成功
   -2为有重复的文件名
   -1为失败，不存在box(creatifnotfound == false时才会返回)
   0为失败，原因未知
*/
int AddFileToBox(ptree& pt_root,string FileName,string BoxName,bool creatifnotfoundbox = false,string otherpath = ""){
	
	try{
		ptree& NodeBox = pt_root.get_child("root.boxs");

		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name") == BoxName){

				try{
					if(SameNameInOneNode(it->second.get_child("files"),FileName,otherpath,true) >= 1){
						return -2;
					}
				}
				catch(...){
				}

				
				return AddFileToFloderNode(it->second,FileName,otherpath);
			}
		}
	}
	catch(...){
	if(creatifnotfoundbox){
		AddNewBox(pt_root,BoxName);
		return AddFileToBox(pt_root,FileName,BoxName,creatifnotfoundbox,otherpath);

	}else{
		return -1;
	}
	}

	return 0;
}



/*
  直接将文件夹添加至指定Box
  in:
   pt root节点地址
   FolderName 要添加的文件夹名
   BoxName 指定Box的名字
   creatifnotfound 如果指定的Box不存在是否创建，默认为不创建
  out:（none)
  return:
   1为成功
   -2为有重复的文件夹
   -1为失败，不存在box(creatifnotfound == false时才会返回)
   0为失败，原因未知
*/
int AddFolderToBox(ptree& pt,string FolderName,string BoxName,bool creatifnotfound = false){
	
	try{
		ptree& NodeBox = pt.get_child("root.boxs");

		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name") == BoxName){
				try{
					if(SameNameInOneNode(it->second.get_child("folders"),FolderName)){
						return -2;
					}
				}
				catch(...){
				}
				ptree NodeFolder;
				NodeFolder.put("name",FolderName);
				NodeFolder.put("numfile",0);
				NodeFolder.put("numfolder",0);
				it->second.put("numfolder",it->second.get<int>("numfolder",0)+1);
				it->second.add_child("folders.folder",NodeFolder);
				return 1;
			}
		}
	}
	catch(...){
	if(creatifnotfound){
		AddNewBox(pt,BoxName);
		if(AddFolderToBox(pt,FolderName,BoxName) == 1){
			return 1;
		}else{
			return 0;
		}

	}else{
		return -1;
	}
	}
	return 0;
}





/*
  通过文件夹节点，将文件添加至文件夹
  in:
	pt_Folder 指定的文件夹节点
	FileName 要添加的文件名
	OtherFullPathInBox 当添加文件与其他box中的文件为同一文件时，该值记录其他box的全路径
	*注意：该函数不会检查OtherFullPathInBox指向的文件是否存在
  out:(none)
  return:
	-2 文件已存在
	1 添加成功
	-1 OtherFullPath与文件名不符
*/
int AddFileToFloderNode(ptree& pt_Folder,string FileName,string OtherFullPathInBox = ""){
	if(OtherFullPathInBox != ""){
		if(GetFileNameFromFullPath(OtherFullPathInBox,"/") != FileName){
			return -1;
		}
	}
	try{
		if(SameNameInOneNode(pt_Folder.get_child("files"),FileName,OtherFullPathInBox,true) >= 1){
			return -2;
		}
	}
	catch(...){
	}

	ptree NodeFile;
	NodeFile.put("name",FileName);
	if(OtherFullPathInBox != ""){
		NodeFile.put("otherpaths.otherpath",OtherFullPathInBox);
	}
	pt_Folder.add_child("files.file",NodeFile);
	pt_Folder.put("numfile",pt_Folder.get<int>("numfile",0)+1);
	//直接添加OtherFullPathInBox
	return 1;
}

/*
  通过文件夹节点，将文件夹添加至文件夹
  in:
	pt_Folder 指定的文件夹节点
	FileName 要添加的文件夹名
  out:(none)
  return:
	-2 文件夹已存在
	1 添加成功
*/
int AddFolderToFloderNode(ptree& pt_Folder,string FolderName){

	try{
		if(SameNameInOneNode(pt_Folder.get_child("folders"),FolderName) ){
			return -2;
		}
	}
	catch(...){
	}

	ptree NodeFolder;
	NodeFolder.put("name",FolderName);
	NodeFolder.put("numfile",0);
	NodeFolder.put("numfolder",0);
	pt_Folder.add_child("folders.folder",NodeFolder);
	pt_Folder.put("numfolder",pt_Folder.get<int>("numfolder",0)+1);
	return 1;
	
}

/*
  替换字符串
  将str中的findstr字符串替换为Tostr
  in:
	str 要更改的字符串
	Tostr 目的字符串
	findstr 要找到的字符串

   out:(none)

   return:
	返回替换后的字符串
*/
string replacestring(string str,string Tostr,string findstr){

	string value = str;
	while(1){
		int pos = value.find(findstr);
		if( pos > 0 && pos < int(value.length())){
			value = value.replace(pos,1,Tostr);
		}else{
			break;
		}
	};

	return value;
}

bool FilePathExistInFolderNode(ptree pt_Folder,string FullPath,bool FunctionInsideValue_NoNeedInput = false){
	
	
	if(FullPath == ""){
		return false;
	}

	//判断最后一位是否为'/'
	if(FullPath.find_last_of("/") == (FullPath.length()-1)){
		if(!FunctionInsideValue_NoNeedInput){
			return false;
		}
	}else{
		FullPath += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPath;
		pos = FullPath.find("/");


		if(pos != FullPath.length()-1){
			replacePath = replacePath.erase(pos);
				//创建分支
			try{
				add = SameNameInOneNode(pt_Folder.get_child("folders"),replacePath);
			}
			catch(...){
				return false; 
			}

			if(add < 1 ){
				return false;
			}

			try{
				ptree& NodeFolder = pt_Folder.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPath;
						return FilePathExistInFolderNode(it->second,replacePath.erase(0,pos+1),true);
						break;
					}
				}
			}
			catch(...){

			}

		}else{
			replacePath = replacePath.erase(pos);
			try{
				if(SameNameInOneNode(pt_Folder.get_child("files"),replacePath) < 1){
					return false;
				}else{
					return true;
				}
			}
			catch(...){
				return false;
			}

		}
		
	
	return false;
}

/*
   看folderFullPath是否在Folder中
      in:
	pt_Folder 指定文件夹(或box)的地址
	FullPath 要添加的文件夹的地址
	*注意:FullPath中不包括指定文件夹的名字
	return:
	true 存在
	false 不存在
*/
bool FolderPathExistInFolderNode(ptree pt_Folder,string FullPath){
	
	
	if(FullPath == ""){
		return false;
	}

	//判断最后一位是否为'/'
	if(FullPath.find_last_of("/") == (FullPath.length()-1)){
		
	}else{
		FullPath += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPath;
		pos = FullPath.find("/");
		replacePath = replacePath.erase(pos);
		try{
			add = SameNameInOneNode(pt_Folder.get_child("folders"),replacePath);
		}
		catch(...){
			return false; 
		}
		
		if(add < 1 ){
			return false;
		}

		if(pos != FullPath.length()-1){
			try{
				ptree& NodeFolder = pt_Folder.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPath;

						return (FolderPathExistInFolderNode(it->second,replacePath.erase(0,pos+1)));
						

						break;
					}
				}
			}
			catch(...){

			}

		}else{

			return true;
		}
		
	
	return false;
}
/*
   将文件夹添加至文件夹(box)节点,如果路径不存在将会被创建
   例如:
	将folder1/folder2/添加至foldermain中
	AddFolderPathToFolderNode( pt_Folder ,"folder1/folder2/")
	pt_Folder为指向文件夹(或box)foldermain的地址
   ========================================================
   in:
	pt_Folder 指定文件夹(或box)的地址
	FullPath 要添加的文件夹的地址
	*注意:FullPath中不包括指定文件夹的名字
   -------------------------------------------------------
   out:(none)
   -------------------------------------------------------
   return:
	0或-1 失败
	1 创建成功
	-2 文件夹已存在
*/
int AddFolderPathToFolderNode(ptree& pt_Folder,string FullPath){
	
	
	if(FullPath == ""){
		return -1;
	}

	//判断最后一位是否为'/'
	if(FullPath.find_last_of("/") == (FullPath.length()-1)){
		
	}else{
		FullPath += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPath;
		pos = FullPath.find("/");
		replacePath = replacePath.erase(pos);
			//创建分支
		add = AddFolderToFloderNode(pt_Folder,replacePath);
		if(add == 0 || add == -1){
			return add;
		}

		if(pos != FullPath.length()-1){
			try{
				ptree& NodeFolder = pt_Folder.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPath;
						add = AddFolderPathToFolderNode(it->second,replacePath.erase(0,pos+1));//!!!!
						if(add == -1 || add == 0){
							return add;
						}

						break;
					}
				}
			}
			catch(...){

			}

		}
		
	
	return add;
}

/*
   将文件添加至文件夹(box)节点,如果路径不存在将会被创建
   例如:
	将folder1/file1添加至foldermain中
	AddFolderPathToFolderNode( pt_Folder ,"folder1/file1")
	pt_Folder为指向文件夹(或box)foldermain的地址
   ========================================================
   in:
	pt_Folder 指定文件夹(或box)的地址
	FullPath 要添加的文件夹的地址
	*注意:FullPath中不包括指定文件夹(或box)的名字
	OtherPath 为其他box中的路径
   -------------------------------------------------------
   out:(none)
   -------------------------------------------------------
   return:
	0或-1 失败
	1 创建成功
	-2 文件已存在

	*注意:FunctionInsideValue_NoNeedInput为函数迭代需要调用的值，使用者不需要赋值
*/
int AddFilePathToFolderNode(ptree& pt_Folder,string FullPath,string OtherPath = "",bool FunctionInsideValue_NoNeedInput = false){
	
	
	if(FullPath == ""){
		return -1;
	}

	//判断最后一位是否为'/'
	if(FullPath.find_last_of("/") == (FullPath.length()-1)){
		if(!FunctionInsideValue_NoNeedInput){
			return -1;
		}
	}else{
		FullPath += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPath;
		pos = FullPath.find("/");


		if(pos != FullPath.length()-1){
			replacePath = replacePath.erase(pos);
				//创建分支
			add = AddFolderToFloderNode(pt_Folder,replacePath);
			if(add == 0 || add == -1){
				return add;
			}


				ptree& NodeFolder = pt_Folder.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPath;
						add = AddFilePathToFolderNode(it->second,replacePath.erase(0,pos+1),OtherPath,true);//!!!!
						if(add == -1 || add == 0){
							return add;
						}

						break;
					}
				}


		}else{
			replacePath = replacePath.erase(pos);
				//创建分支
			add = AddFileToFloderNode(pt_Folder,replacePath,OtherPath);
			return add;

		}
		
	
	return 1;
}

/*
 将FolderPath的内容添加进Root
 例如:将boxname/foldername1/foldername2/添加进root
 =================================================
 in:
	pt_root 根地址
	FolderPath 为添加文件夹全地址
		*注意：FolderPath第一位为BoxNmae
	CreatIfNotFound 如果目录不存在是否创建 默认不创建
 --------------------------------------------------
 out:(none)

 return:
	 -1或0为创建失败
	 1为创建成功
	 -2为路径已存在
*/
int AddFolderPathToRoot(ptree& pt_root,string FolderPath){

	if(FolderPath == ""){
		return -1;
	}


	int pos = FolderPath.find("/");
	string str;

	
	if(pos<0 || pos >= int(FolderPath.length()-1)){//只有boxname
		return AddNewBox(pt_root,FolderPath);
	}else{
		str = FolderPath;
		AddNewBox(pt_root,str.erase(pos));
		for(auto it=pt_root.get_child("root.boxs").begin();it != pt_root.get_child("root.boxs").end();++it){
			if(it->second.get("name","") == str){
				str = FolderPath;
				return AddFolderPathToFolderNode(it->second,str.erase(0,pos+1));
			}
		}
		return 0;

	}

}

/*
 将FilePath的内容添加进Root
 例如:将boxname/foldername1/file1添加进root
 =================================================
 in:
	pt_root 根地址
	FilePath 为添加文件夹全地址
		*注意：FilePath第一位为BoxNmae
	
 --------------------------------------------------
 out:(none)

 return:
	 -1或0为创建失败
	 1为创建成功
	 -2为路径已存在
*/
int AddFilePathToRoot(ptree& pt_root,string FilePath,string OtherPath = ""){

	if(FilePath == ""){
		return -1;
	}
	if(FilePath == OtherPath){
		OtherPath = "";
	}

	int pos = FilePath.find("/");
	string str;

	if(FilePath.find_last_of("/") > (FilePath.length()-1)){

		return -1;
	}else{

		if(OtherPath != ""){
			if(GetFileNameFromFullPath(FilePath,"/") == GetFileNameFromFullPath(OtherPath,"/")){

				//判断OtherPath是否存在
				try{
					for(auto it = pt_root.get_child("root.boxs").begin();it != pt_root.get_child("root.boxs").end();++it){
						if(it->second.get<string>("name") == GetBoxNameFromFullPathInBox(OtherPath)){
							if(!(FilePathExistInFolderNode(it->second,OtherPath.substr(OtherPath.find("/")+1)))){
								return -1;//otherPath不存在
							}else{
								AddFilePathToRoot(pt_root,OtherPath,FilePath);
							}
						}
					}
				}
				catch(...){
					return -1;//otherPath不存在
				}
			}else{
				return -1;
			}
		}

		if(FilePath.find("/") == FilePath.length()-1){//只有boxname
		
			return AddNewBox(pt_root,FilePath);
		}else{
			str = FilePath;
			AddNewBox(pt_root,str.erase(pos));
			for(auto it=pt_root.get_child("root.boxs").begin();it != pt_root.get_child("root.boxs").end();++it){
				if(it->second.get("name","") == str){
					str = FilePath;
					return AddFilePathToFolderNode(it->second,str.erase(0,pos+1),OtherPath);
				}
			}
			return 0;
		}
	}

}

/*
   替代boost中的write_xml函数
   in:
	FullPath 要写入的xml的全路径,如果路径不存在将创建路径
	pt 要写的ptree
   out:(none)
   return:
	-1 创建失败，FullPath格式不对
	1 创建成功
*/
int write_xml_CHECKPATH(string FullPath,ptree pt){

	string ToPath = FullPath;

	int pos = ToPath.find(".xml");
	if(pos != ToPath.length()-4){
		return -1;
	}
	//win
	pos = ToPath.find_last_of("\\");
	if(pos < int(ToPath.length())-1 && pos >= 0){


		ToPath = FullPath.substr(0,pos);

		system(("mkdir " + ToPath + ">nul 2>nul" ).c_str());

	}

	write_xml(FullPath,pt);
	return 1;
}

/*
   替代boost中的read_xml函数
   in:
	FullPath 要写入的xml的全路径,如果路径不存在读取失败
	pt 要读入的ptree
   out:
	pt 读后的
   return:
	-1 文件不存在
	1 创建成功
*/
int read_xml_CHECKPATH(string FullPath,ptree& pt){

	
	
	_finddata_t file;
	long lf;
	if ((lf = _findfirst( FullPath.c_str(), &file)) == -1l) {

		return -1;
	}else{
		read_xml(FullPath,pt);
	}
	_findclose(lf);
	return 1;
}

/*
	将box输入到xml中
	in:
		pt_root 根节点
		BoxName Box名字
		FullPathToWrite 要写入.xml文件的全地址
	out:(none)
	return:
		-1 失败
		1 成功
*/
int write_box_xml(ptree pt_root,string BoxName,string FullPathToWrite){
	try{
		ptree BoxsNode = pt_root.get_child("root.boxs");
		
		for(auto it=BoxsNode.begin();it != BoxsNode.end();++it){
			if(it->second.get("name","") == BoxName){
				ptree BoxNode;
				BoxName.clear();
				BoxNode.add_child("box",it->second);
				return write_xml_CHECKPATH(FullPathToWrite,BoxNode);
				break;
			}
		}
	}
	catch(...){

	}
	return -1;
}

ptree read_box_xml(string BoxName,string FullPathToRead){

	ptree pt_box;
	pt_box.clear();
	read_xml_CHECKPATH("FullPathToRead",pt_box);
	try{
		pt_box.get_child("box");
	}
	catch(...){

	}

	return pt_box;
}

int AddOtherPathToFileOnBoxNode(ptree & BoxNode,string FullPathInBox,string OtherFullPathInBox,bool FunctionInsideValue_NoNeedInput = false){

	if(FullPathInBox == ""){
		return -1;
	}

	//判断最后一位是否为'/'
	if(FullPathInBox.find_last_of("/") == (FullPathInBox.length()-1)){
		if(!FunctionInsideValue_NoNeedInput){
			return -1;
		}
	}else{
		FullPathInBox += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPathInBox;
		pos = FullPathInBox.find("/");


		if(pos != FullPathInBox.length()-1){
			replacePath = replacePath.erase(pos);
				//创建分支
			add = AddFolderToFloderNode(BoxNode,replacePath);
			if(add == 0 || add == -1){
				return add;
			}

			try{
				ptree& NodeFolder = BoxNode.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPathInBox;
						add = AddFilePathToFolderNode(it->second,replacePath.erase(0,pos+1),OtherFullPathInBox,true);//!!!!
						if(add == -1 || add == 0){
							return add;
						}

						break;
					}
				}
			}
			catch(...){

			}

		}else{
			replacePath = replacePath.erase(pos);
				//创建分支
			add = AddFileToFloderNode(BoxNode,replacePath);

			if(add <= 0){
				return add;
			}
			if(add == 2){
				//需要检查path有没有
			}

		}
		
	
	return 1;
}



void RenamedFolderPathInNode(ptree& ptNode,string OldPath, string NewPath,string itor=""){
	bool lastfolder = false;
	if(itor != "" && itor.find("/") >= int(itor.length())){
		lastfolder = true;
	}

		try{
			for(auto it = ptNode.get_child("files").begin();it != ptNode.get_child("files").end();++it){
				try{

					for(auto it2 = it->second.get_child("otherpaths").begin();
						it2 != it->second.get_child("otherpaths").end();++it2){
							string str = it2->second.get_value<string>();

							if(str.find(OldPath) < str.length()-unsigned(1)){
								it2->second.put_value(NewPath+str.substr(OldPath.length()));
							}
					}
				}
				catch(...){
				}
			}
		}
		
		catch(...){

		}

		try{
			for(auto it = ptNode.get_child("folders").begin();it != ptNode.get_child("folders").end();++it){
				if(lastfolder && it->second.get<string>("name","") == itor){
					it->second.put("name",GetFileNameFromFullPath(NewPath,"/"));
					RenamedFolderPathInNode(it->second,OldPath,NewPath);
				}else if(itor != "" && (!lastfolder) && it->second.get<string>("name","") == GetBoxNameFromFullPathInBox(itor)){
					RenamedFolderPathInNode(it->second,OldPath,NewPath,itor.substr(itor.find("/")+1));
				}else{
					RenamedFolderPathInNode(it->second,OldPath,NewPath);
				}
			}
		}
		
		catch(...){

		}
	
}


/*
   将Box中的文件夹更改名字
*/
int RenamedFolderPathInRoot(ptree& pt_root,string OldPath, string NewPath){
		
	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//判断最后一位是否为'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		OldPath.erase(OldPath.length()-1);
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		NewPath.erase(NewPath.length()-1);
	}else{

	}

	//判断一下是否只有最后一个文件名字不同
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}
	bool lastfolder = false;
	string itor = OldPath;
	if(itor != "" && itor.find("/") >= int(itor.length())){
		lastfolder = true;
	}
	try{
		for(auto it = pt_root.get_child("root.boxs").begin(); it != pt_root.get_child("root.boxs").end();++it){

			
				if(lastfolder && it->second.get<string>("name","") == itor){
					it->second.put("name",GetFileNameFromFullPath(NewPath,"/"));
					RenamedFolderPathInNode(it->second,OldPath,NewPath);
				}else if(itor != "" && (!lastfolder) && it->second.get<string>("name","") == GetBoxNameFromFullPathInBox(itor)){
					RenamedFolderPathInNode(it->second,OldPath,NewPath,itor.substr(itor.find("/")+1));
				}else{
					RenamedFolderPathInNode(it->second,OldPath,NewPath);
				}
		}
	}
	catch(...){
		return -1;
	}
	return 1;
	
}

void RenamedFilePathInNode(ptree& ptNode,string OldPath, string NewPath ,string itor = ""){

	bool InFileFolder = false;
	if(itor != "" && GetFileNameFromFullPath(itor,"/") == itor){
		InFileFolder = true;
	}

		try{
			for(auto it = ptNode.get_child("files").begin();it != ptNode.get_child("files").end();++it){
				try{
						if(InFileFolder && it->second.get<string>("name") == itor){
							it->second.put("name",GetFileNameFromFullPath(NewPath,"/"));
							for(auto it2 = it->second.get_child("otherpaths").begin();
							it2 != it->second.get_child("otherpaths").end();++it2){
								string str = it2->second.get_value<string>();

								it2->second.put_value(
									OldPath.substr(0,OldPath.find_last_of("/")+1)
									+GetFileNameFromFullPath(NewPath,"/")
									);
							}
							
						}else{

							bool filenameneedchange = false;
						
							for(auto it2 = it->second.get_child("otherpaths").begin();
								it2 != it->second.get_child("otherpaths").end();++it2){
									string str = it2->second.get_value<string>();

									if(str == OldPath){
										filenameneedchange = true;
										break;
									}
							}
							if(filenameneedchange){
								it->second.put("name",GetFileNameFromFullPath(NewPath,"/"));
								for(auto it2 = it->second.get_child("otherpaths").begin();
								it2 != it->second.get_child("otherpaths").end();++it2){
									string str = it2->second.get_value<string>();

									it2->second.put_value(
										OldPath.substr(0,OldPath.find_last_of("/")+1)
										+GetFileNameFromFullPath(NewPath,"/")
										);
								}
							}
						}
				}
				catch(...){
				}
			}
		}
		
		catch(...){

		}

		try{
			for(auto it = ptNode.get_child("folders").begin();it != ptNode.get_child("folders").end();++it){
				if(itor == ""){
					RenamedFilePathInNode(it->second,OldPath,NewPath);
				}else if(InFileFolder){
					RenamedFilePathInNode(it->second,OldPath,NewPath);
				}else{
					RenamedFilePathInNode(it->second,OldPath,NewPath,itor.substr(itor.find("/")+1));
				}
			}
		}
		
		catch(...){

		}
	
}

/*
   将Box中的文件更改名字
*/
int RenamedFilePathInRoot(ptree& pt_root,string OldPath, string NewPath){
		
	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//判断最后一位是否为'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		return -1;
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		return -1;
	}else{

	}

	//判断一下是否只有最后一个文件名字不同
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}


	try{
		for(auto it = pt_root.get_child("root.boxs").begin(); it != pt_root.get_child("root.boxs").end();++it){
			if(it->second.get<string>("name","") == GetBoxNameFromFullPathInBox(OldPath)){
					RenamedFilePathInNode(it->second,OldPath,NewPath,OldPath.substr(OldPath.find("/")+1));
			}else{
				RenamedFilePathInNode(it->second,OldPath,NewPath);
			}
				
		}
	}
	catch(...){

	}
	return 1;
	
}

int FindPtreeAdressByFullPath(ptree & pt_root,string FileFullPath,ptree * needptree){

	ptree * pptree;
	try{
		for(auto itbox = pt_root.get_child("root.boxs").begin();itbox != pt_root.get_child("root.boxs").end();++itbox){
			if(itbox->second.get<string>("name") == GetBoxNameFromFullPathInBox(FileFullPath)){
				pptree = &(itbox->second);
				FileFullPath.erase(0,FileFullPath.find("/")+1);
				unsigned int length_pre(-1),length(FileFullPath.length());
				if(FileFullPath == GetFileNameFromFullPath(FileFullPath,"/")){
					return -1;
				}
				
				while(1){
					if(length_pre == length){
						return -1;
					}
					if(FileFullPath.find("/") >= FileFullPath.length()){//已到达最后一个文件夹
						try{
							for(auto itfile = (*pptree).get_child("files").begin();itfile != (*pptree).get_child("files").end();++itfile){
								if(itfile->second.get<string>("name","") == FileFullPath){
									//找到
									needptree = &(itfile->second);
									return 1;
								}
							}
						}
						catch(...){
							(*needptree).clear();
							return -1;
						}
					}else{
						try{
							for(auto itfolder=(*pptree).get_child("folders").begin();
								itfolder !=(*pptree).get_child("folders").end();++itfolder){
									if(itfolder->second.get<string>("name") == GetBoxNameFromFullPathInBox(FileFullPath)){
										pptree = &(itfolder->second);
										FileFullPath.erase(0,FileFullPath.find("/")+1);
										break;
									}
							}
						}
						catch(...){
							
							return -1;
						}

					}
					length_pre = length;
					length = FileFullPath.length();
				}

			}
		}

	}
	catch(...){
		
		return -1;
	}
}
}
#endif
 