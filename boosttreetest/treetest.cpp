#include <stdio.h>  
  
#include <iostream>  
#include <sstream>  
#include <string>  
#include <locale>  
  
#include "boost/property_tree/ptree.hpp"  
#include "boost/property_tree/json_parser.hpp"  
#include "boost/property_tree/ini_parser.hpp"  
#include "boost/property_tree/xml_parser.hpp"  
  
//#include "boost/program_options/detail/convert.hpp"  
//#include "boost/program_options/detail/utf8_codecvt_facet.hpp"  
 
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


//判断在同一节点下是否存name的文件（夹）
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

//将File添加至指定BOX
//out
/*
1为成功
-2为有重复的文件名
-1为失败，不存在box
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


//return -2为文件已存在
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



//out
/*
1为成功
-2为有文件夹已存在
-1为失败，不存在box
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

//-2为已存在
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

//将文件Filename添加至文件夹pt_Folder节点上
//返回值:
//-2为文件已存在
//1为成功
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


//通过文件夹路径创建文件夹
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
 out:
 返回整数型
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

int AddFilePathToRoot(ptree& pt_root,string FolderPath){

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
				return AddFilePathToFolderNode(it->second,str.erase(0,pos+1));
			}
		}
		return 0;

	}

}

 int main(void)  
{    
	    //std::wifstream f("miniWidget.xml");
		//std::locale utf8Locale(locale(), new codecvt_utf8<wchar_t>);
        //f.imbue(utf8Locale); //先转换一下   
		//AddNewBox(pt,"box1");
		ptree pt;
		AddFolderPathToRoot(pt,"box1");
		write_xml("out.xml",pt);
		pt.clear();
		read_xml("out.xml",pt);
		for(auto it=pt.begin();it!=pt.end();++it){
			cout<<it->second.data()<<endl;
		}

        return 0;  
 }    