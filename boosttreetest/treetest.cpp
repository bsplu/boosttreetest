#include <iostream>  
#include <string>  
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>  
#include <boost/foreach.hpp>  
 
 using namespace std;
 using namespace boost::property_tree;


void printTree(ptree root, int curLayer)
{  
        BOOST_FOREACH(ptree::value_type &v1, root){  
  
            if(v1.first == "<xmlattr>"){  
                BOOST_FOREACH(ptree::value_type &vAttr, v1.second){  
  
                    int layer = curLayer;  
                    while (layer--)  
                    {  
                        cout<<'\t';  
                    }  
  
                    cout<<vAttr.first<<"="<<vAttr.second.data()<<endl;  
                }  
            }  
            else  
            {  
  
                int layer = curLayer;  
                while (layer--)  
                {  
                    cout<<'\t';  
                }  
  
                cout<<v1.first;  
  
                if (v1.second.empty() == true)  
                {  
                    cout<<"="<<v1.second.data();  
                }  
                cout<<endl;  
                if (v1.second.empty() == false)  
                {  
                    printTree(v1.second, curLayer+1);  
                }  
            }  
        }  
    }  
    int main(void)  
    {    
        ptree pt;    
        read_xml("miniWidget.xml", pt);  
  
        printTree(pt, 0);  
        return 0;  
    }    