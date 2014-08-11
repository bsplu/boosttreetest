#ifndef FILE_TREE_HPP_INCLUDE
#define FILE_TREE_HPP_INCLUDE

#include <stdio.h>  
  
#include <iostream>  
#include <sstream>  
#include <string>  
#include <locale>  
  
#include "boost/property_tree/ptree.hpp"  
#include "boost/property_tree/json_parser.hpp"  
#include "boost/property_tree/ini_parser.hpp"  
#include "boost/property_tree/xml_parser.hpp"  

 
 using namespace std;
 using namespace boost::property_tree;

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
  判断在同一节点下是否存name的文件（夹）
  in:
  pt 判断的节点，文件夹节点("folders")或文件节点("files")
  name 要判断的文件(夹)的名

  out:(none)

  return:
  true 存在
  false 不存在
*/
bool SameNameInOneNode(ptree pt,string name){

	if(pt.empty() == true){
		return false;
	}else{
		for(auto it = pt.begin();it != pt.end();++it){
			if(it->second.get<string>("name","") == name){
				return true;
			}
		}
		return false;
	}

}

/*
  直接将文件添加至指定Box
  in:
   pt_root root节点地址
   FileName 要添加文件的文件名
   BoxName 指定Box的名字
   creatifnotfound 如果指定的Box不存在是否创建，默认为不创建
  out:（none)
  return:
   1为成功
   -2为有重复的文件名
   -1为失败，不存在box(creatifnotfound == false时才会返回)
   0为失败，原因未知
*/
int AddFileToBox(ptree& pt_root,string FileName,string BoxName,bool creatifnotfound = false){
	
	try{
		ptree& NodeBox = pt_root.get_child("root.boxs");

		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name") == BoxName){

				try{
					if(SameNameInOneNode(it->second.get_child("files"),FileName)){
						return -2;
					}
				}
				catch(...){
				}

				ptree NodeFile;
				NodeFile.put("name",FileName);
				it->second.put("numfile",it->second.get<int>("numfile",0)+1);
				it->second.add_child("files.file",NodeFile);
				return 1;
			}
		}
	}
	catch(...){
	if(creatifnotfound){
		AddNewBox(pt_root,BoxName);
		if(AddFileToBox(pt_root,FileName,BoxName) == 1){
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
  直接将文件添加至指定Box
  in:
   pt_box box节点地址
   FileName 要添加文件的文件名
   BoxName 指定Box的名字
   creatifnotfound 如果指定的Box不存在是否创建，默认为不创建
  out:（none)
  return:
   1为成功
   -2为有重复的文件名
   -1为失败，不存在box(creatifnotfound == false时才会返回)
   0为失败，原因未知
*/
int AddFileToBox(ptree& pt_box,string FileName){


	try{
		if(SameNameInOneNode(pt_box.get_child("files"),FileName)){
			return -2;
		}
	}
	catch(...){
	}

	pt_box.put("numfile",pt_box.get<int>("numfile",0)+1);
	ptree NodeFile;
	NodeFile.put("name",FileName);
	pt_box.add_child("files.file",NodeFile);

	return 1;

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
  直接将文件夹添加至指定Box
  in:
   pt_box box节点地址
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
int AddFolderToBox(ptree& pt_box,string FolderName){


	try{
		if(SameNameInOneNode(pt_box.get_child("folders"),FolderName)){
			return -2;
		}
	}
	catch(...){
	}

	pt_box.put("numfolder",pt_box.get<int>("numfolder",0)+1);
	ptree NodeFile;
	NodeFile.put("name",FolderName);
	pt_box.add_child("files.file",NodeFile);

	return 1;

}

/*
  通过文件夹节点，将文件添加至文件夹
  in:
	pt_Folder 指定的文件夹节点
	FileName 要添加的文件名
  out:(none)
  return:
	-2 文件已存在
	1 添加成功
*/
int AddFileToFloderNode(ptree& pt_Folder,string FileName){
	try{
		if(SameNameInOneNode(pt_Folder.get_child("files"),FileName) ){
			return -2;
		}
	}
	catch(...){
	}

	ptree NodeFile;
	NodeFile.put("name",FileName);
	pt_Folder.add_child("files.file",NodeFile);
	pt_Folder.put("numfile",pt_Folder.get<int>("numfile",0)+1);
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
   -------------------------------------------------------
   out:(none)
   -------------------------------------------------------
   return:
	0或-1 失败
	1 创建成功
	-2 文件夹已存在

	*注意:FunctionInsideValue_NoNeedInput为函数迭代需要调用的值，使用者不需要赋值
*/
int AddFilePathToFolderNode(ptree& pt_Folder,string FullPath,bool FunctionInsideValue_NoNeedInput = false){
	
	
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

			try{
				ptree& NodeFolder = pt_Folder.get_child("folders");

				for(auto it = NodeFolder.begin();it != NodeFolder.end();++it){
					if(it->second.get<string>("name",replacePath) == replacePath){
						//pos = FullPath.find("/",pos+1);
						replacePath = FullPath;
						add = AddFilePathToFolderNode(it->second,replacePath.erase(0,pos+1),true);//!!!!
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
			add = AddFileToFloderNode(pt_Folder,replacePath);
			if(add <= 0){
				return add;
			}

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
	CreatIfNotFound 如果目录不存在是否创建 默认不创建
 --------------------------------------------------
 out:(none)

 return:
	 -1或0为创建失败
	 1为创建成功
	 -2为路径已存在
*/
int AddFilePathToRoot(ptree& pt_root,string FilePath){

	if(FilePath == ""){
		return -1;
	}


	int pos = FilePath.find("/");
	string str;

	
	if(pos<0 || pos >= int(FilePath.length()-1)){//只有boxname
		return AddNewBox(pt_root,FilePath);
	}else{
		str = FilePath;
		AddNewBox(pt_root,str.erase(pos));
		for(auto it=pt_root.get_child("root.boxs").begin();it != pt_root.get_child("root.boxs").end();++it){
			if(it->second.get("name","") == str){
				str = FilePath;
				return AddFilePathToFolderNode(it->second,str.erase(0,pos+1));
			}
		}
		return 0;

	}

}

#endif
 