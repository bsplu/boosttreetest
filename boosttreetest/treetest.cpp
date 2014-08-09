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

	}
	catch(...){
		cout<<"没找到root.boxs"<<endl;
	}
	//检查boxname是否有重复

		


	pt.put("root.numbox",pt.get<int>("root.numbox",0)+1);
	
	
	NodeBox.clear();
	NodeBox.put("name",boxname);
	NodeBox.put("numfile",0);
	NodeBox.put("numfolder",0);
	
	pt.add_child("root.boxs.box",NodeBox);


	return 1;
	
}

//将File添加至指定BOX
int AddFileToBox(ptree& pt,string FileName,string BoxName,bool creatifnotfound = false){
	
	try{
		ptree& NodeBox = pt.get_child("root.boxs");

		for(auto it = NodeBox.begin();it != NodeBox.end();++it){
			if(it->second.get<string>("name") == BoxName){
				ptree NodeFile;
				NodeFile.put("file",FileName);
				it->second.put("numfile",it->second.get<int>("numfile",0)+1);
				it->second.add_child("files.file",NodeFile);
				return 1;
			}
		}
	}
	catch(...){
	if(creatifnotfound){
		AddNewBox(pt,BoxName);
		if(AddFileToBox(pt,FileName,BoxName) == 1){
			return 1;
		}else{
			return 0;
		}

	}else{
		return -1;
	}
	}
}

 int main(void)  
{    
	    //std::wifstream f("miniWidget.xml");
		//std::locale utf8Locale(locale(), new codecvt_utf8<wchar_t>);
        //f.imbue(utf8Locale); //先转换一下   
		//AddNewBox(pt,"box1");
		ptree pt;
		//AddNewBox(pt,"box1");
		if(AddFileToBox(pt,"file1","box1",true) == 0){
			cout<<"erro unknown"<<endl;
		}
		//cout<<(*ptest).get<string>("name")<<endl;
		write_xml("out.xml",pt);
        return 0;  
 }    