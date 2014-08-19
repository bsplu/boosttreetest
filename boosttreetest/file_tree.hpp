#ifndef FILE_TREE_HPP_INCLUDE
#define FILE_TREE_HPP_INCLUDE

#include <stdio.h>  
  
#include <iostream>  
#include <sstream>  
#include <string>  
#include <locale>  
#include <direct.h> //����Ŀ¼


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
   ��str_root�е�Oldestr�滻ΪNewstr
*/
string ReplaceString(string str_root,string Oldstr,string Newstr){

	int pos = str_root.find(Oldstr);
	if(pos >= int(str_root.length())){
		return str_root;
	}

	return str_root.replace(pos,Oldstr.length(),Newstr);
}


/*
   ��ȫ·����ȡ��box��
   ����:���ȫ·��"box1/folder1/file.txt"�õ�"box1"
        ��GetBoxNameFromFullPathInBox("box1/folder1/file.txt");
   ============================================================
   in:
	FullPathInBox Ҫ��ȡ��ȫ·��
   ------------------------------------------------------------
   out:(none)
   ------------------------------------------------------------
   return:
	box����
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
   ��ȫ·����ȡ���ļ���
   ����:���ȫ·��"folder1\folder2\file.txt"�õ�"file.txt"
        ��GetFileNameFromFullPath("folder1\folder2\file.txt");
   ============================================================
   in:
	FullPath Ҫ��ȡ��ȫ·��
   ------------------------------------------------------------
   out:(none)
   ------------------------------------------------------------
   return:
	�ļ���
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
	if(rNode.second.empty() == true){//���ٺ��з�֧
		
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
  �½�һ��box,������Ϊboxname
  in:
  pt root�ĵ�ַ
  boxname ��box������
  out:(none)
  return:
  -1 ����ʧ��
  -2 �Ѵ���boxname
  1 �����ɹ�
*/
int AddNewBox(ptree& pt,string boxname){
	//����Ƿ���root
	ptree NodeBox;

	try{

		NodeBox = pt.get_child("root.boxs");
		//���boxname�Ƿ��ظ�
		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name","") == boxname){
				return -2;
			}
		}
	}
	catch(...){
		cout<<"û�ҵ�root.boxs"<<endl;
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
  �ж����ļ��ڵ����Ƿ��otherpath
  in:
  pt �жϵĽڵ㣬�ļ��ڵ�
  otherpath �ж�otherpath�Ƿ����
  CreatOtherPathInNoexist ���otherpath�������Ƿ񴴽�

  out:(none)

  return:

  true ����
  false ������

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
  �ж���ͬһ�ڵ����Ƿ��name���ļ����У�
  in:
  pt �жϵĽڵ㣬�ļ��нڵ�("folders")���ļ��ڵ�("files")
  name Ҫ�жϵ��ļ�(��)����
  otherpath �ж�otherpath�Ƿ����
  CreatOtherPathInNoexist ���otherpath�������Ƿ񴴽�

  out:(none)

  return:
  2 ���ڣ���otherpath����
  1 ���ڣ�otherpath������
  -1 ������

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
  ֱ�ӽ��ļ������ָ��Box
  in:
   pt_root root�ڵ��ַ
   FileName Ҫ����ļ����ļ���
   BoxName ָ��Box������
   creatifnotfound ���ָ���������Ƿ񴴽���Ĭ��Ϊ������
  out:��none)
  return:
   1Ϊ�ɹ�
   -2Ϊ���ظ����ļ���
   -1Ϊʧ�ܣ�������box(creatifnotfound == falseʱ�Ż᷵��)
   0Ϊʧ�ܣ�ԭ��δ֪
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
  ֱ�ӽ��ļ��������ָ��Box
  in:
   pt root�ڵ��ַ
   FolderName Ҫ��ӵ��ļ�����
   BoxName ָ��Box������
   creatifnotfound ���ָ����Box�������Ƿ񴴽���Ĭ��Ϊ������
  out:��none)
  return:
   1Ϊ�ɹ�
   -2Ϊ���ظ����ļ���
   -1Ϊʧ�ܣ�������box(creatifnotfound == falseʱ�Ż᷵��)
   0Ϊʧ�ܣ�ԭ��δ֪
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
  ͨ���ļ��нڵ㣬���ļ�������ļ���
  in:
	pt_Folder ָ�����ļ��нڵ�
	FileName Ҫ��ӵ��ļ���
	OtherFullPathInBox ������ļ�������box�е��ļ�Ϊͬһ�ļ�ʱ����ֵ��¼����box��ȫ·��
	*ע�⣺�ú���������OtherFullPathInBoxָ����ļ��Ƿ����
  out:(none)
  return:
	-2 �ļ��Ѵ���
	1 ��ӳɹ�
	-1 OtherFullPath���ļ�������
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
	//ֱ�����OtherFullPathInBox
	return 1;
}

/*
  ͨ���ļ��нڵ㣬���ļ���������ļ���
  in:
	pt_Folder ָ�����ļ��нڵ�
	FileName Ҫ��ӵ��ļ�����
  out:(none)
  return:
	-2 �ļ����Ѵ���
	1 ��ӳɹ�
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
  �滻�ַ���
  ��str�е�findstr�ַ����滻ΪTostr
  in:
	str Ҫ���ĵ��ַ���
	Tostr Ŀ���ַ���
	findstr Ҫ�ҵ����ַ���

   out:(none)

   return:
	�����滻����ַ���
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

	//�ж����һλ�Ƿ�Ϊ'/'
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
				//������֧
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
   ��folderFullPath�Ƿ���Folder��
      in:
	pt_Folder ָ���ļ���(��box)�ĵ�ַ
	FullPath Ҫ��ӵ��ļ��еĵ�ַ
	*ע��:FullPath�в�����ָ���ļ��е�����
	return:
	true ����
	false ������
*/
bool FolderPathExistInFolderNode(ptree pt_Folder,string FullPath){
	
	
	if(FullPath == ""){
		return false;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
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
   ���ļ���������ļ���(box)�ڵ�,���·�������ڽ��ᱻ����
   ����:
	��folder1/folder2/�����foldermain��
	AddFolderPathToFolderNode( pt_Folder ,"folder1/folder2/")
	pt_FolderΪָ���ļ���(��box)foldermain�ĵ�ַ
   ========================================================
   in:
	pt_Folder ָ���ļ���(��box)�ĵ�ַ
	FullPath Ҫ��ӵ��ļ��еĵ�ַ
	*ע��:FullPath�в�����ָ���ļ��е�����
   -------------------------------------------------------
   out:(none)
   -------------------------------------------------------
   return:
	0��-1 ʧ��
	1 �����ɹ�
	-2 �ļ����Ѵ���
*/
int AddFolderPathToFolderNode(ptree& pt_Folder,string FullPath){
	
	
	if(FullPath == ""){
		return -1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
	if(FullPath.find_last_of("/") == (FullPath.length()-1)){
		
	}else{
		FullPath += "/";
	}
	
	int pos = -1;
	int add = 0;
		string replacePath = FullPath;
		pos = FullPath.find("/");
		replacePath = replacePath.erase(pos);
			//������֧
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
   ���ļ�������ļ���(box)�ڵ�,���·�������ڽ��ᱻ����
   ����:
	��folder1/file1�����foldermain��
	AddFolderPathToFolderNode( pt_Folder ,"folder1/file1")
	pt_FolderΪָ���ļ���(��box)foldermain�ĵ�ַ
   ========================================================
   in:
	pt_Folder ָ���ļ���(��box)�ĵ�ַ
	FullPath Ҫ��ӵ��ļ��еĵ�ַ
	*ע��:FullPath�в�����ָ���ļ���(��box)������
	OtherPath Ϊ����box�е�·��
   -------------------------------------------------------
   out:(none)
   -------------------------------------------------------
   return:
	0��-1 ʧ��
	1 �����ɹ�
	-2 �ļ��Ѵ���

	*ע��:FunctionInsideValue_NoNeedInputΪ����������Ҫ���õ�ֵ��ʹ���߲���Ҫ��ֵ
*/
int AddFilePathToFolderNode(ptree& pt_Folder,string FullPath,string OtherPath = "",bool FunctionInsideValue_NoNeedInput = false){
	
	
	if(FullPath == ""){
		return -1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
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
				//������֧
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
				//������֧
			add = AddFileToFloderNode(pt_Folder,replacePath,OtherPath);
			return add;

		}
		
	
	return 1;
}

/*
 ��FolderPath��������ӽ�Root
 ����:��boxname/foldername1/foldername2/��ӽ�root
 =================================================
 in:
	pt_root ����ַ
	FolderPath Ϊ����ļ���ȫ��ַ
		*ע�⣺FolderPath��һλΪBoxNmae
	CreatIfNotFound ���Ŀ¼�������Ƿ񴴽� Ĭ�ϲ�����
 --------------------------------------------------
 out:(none)

 return:
	 -1��0Ϊ����ʧ��
	 1Ϊ�����ɹ�
	 -2Ϊ·���Ѵ���
*/
int AddFolderPathToRoot(ptree& pt_root,string FolderPath){

	if(FolderPath == ""){
		return -1;
	}


	int pos = FolderPath.find("/");
	string str;

	
	if(pos<0 || pos >= int(FolderPath.length()-1)){//ֻ��boxname
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
 ��FilePath��������ӽ�Root
 ����:��boxname/foldername1/file1��ӽ�root
 =================================================
 in:
	pt_root ����ַ
	FilePath Ϊ����ļ���ȫ��ַ
		*ע�⣺FilePath��һλΪBoxNmae
	
 --------------------------------------------------
 out:(none)

 return:
	 -1��0Ϊ����ʧ��
	 1Ϊ�����ɹ�
	 -2Ϊ·���Ѵ���
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

				//�ж�OtherPath�Ƿ����
				try{
					for(auto it = pt_root.get_child("root.boxs").begin();it != pt_root.get_child("root.boxs").end();++it){
						if(it->second.get<string>("name") == GetBoxNameFromFullPathInBox(OtherPath)){
							if(!(FilePathExistInFolderNode(it->second,OtherPath.substr(OtherPath.find("/")+1)))){
								return -1;//otherPath������
							}else{
								AddFilePathToRoot(pt_root,OtherPath,FilePath);
							}
						}
					}
				}
				catch(...){
					return -1;//otherPath������
				}
			}else{
				return -1;
			}
		}

		if(FilePath.find("/") == FilePath.length()-1){//ֻ��boxname
		
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
   ���boost�е�write_xml����
   in:
	FullPath Ҫд���xml��ȫ·��,���·�������ڽ�����·��
	pt Ҫд��ptree
   out:(none)
   return:
	-1 ����ʧ�ܣ�FullPath��ʽ����
	1 �����ɹ�
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
   ���boost�е�read_xml����
   in:
	FullPath Ҫд���xml��ȫ·��,���·�������ڶ�ȡʧ��
	pt Ҫ�����ptree
   out:
	pt �����
   return:
	-1 �ļ�������
	1 �����ɹ�
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
	��box���뵽xml��
	in:
		pt_root ���ڵ�
		BoxName Box����
		FullPathToWrite Ҫд��.xml�ļ���ȫ��ַ
	out:(none)
	return:
		-1 ʧ��
		1 �ɹ�
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

	//�ж����һλ�Ƿ�Ϊ'/'
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
				//������֧
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
				//������֧
			add = AddFileToFloderNode(BoxNode,replacePath);

			if(add <= 0){
				return add;
			}
			if(add == 2){
				//��Ҫ���path��û��
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
   ��Box�е��ļ��и�������
*/
int RenamedFolderPathInRoot(ptree& pt_root,string OldPath, string NewPath){
		
	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		OldPath.erase(OldPath.length()-1);
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		NewPath.erase(NewPath.length()-1);
	}else{

	}

	//�ж�һ���Ƿ�ֻ�����һ���ļ����ֲ�ͬ
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
   ��Box�е��ļ���������
*/
int RenamedFilePathInRoot(ptree& pt_root,string OldPath, string NewPath){
		
	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		return -1;
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		return -1;
	}else{

	}

	//�ж�һ���Ƿ�ֻ�����һ���ļ����ֲ�ͬ
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
					if(FileFullPath.find("/") >= FileFullPath.length()){//�ѵ������һ���ļ���
						try{
							for(auto itfile = (*pptree).get_child("files").begin();itfile != (*pptree).get_child("files").end();++itfile){
								if(itfile->second.get<string>("name","") == FileFullPath){
									//�ҵ�
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
 