#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++) {
	  nd[i]->printTable();
	}
}

void routingAlgo(vector<RoutingNode*> nd){
  //Your code here
    vector<struct routingtbl> routing_table;
    int n=nd.size();
    bool flag=false;
    while(!flag)
    {
        flag=true;//true if tables are not changed in an iteration yet
        routing_table.clear();

        for(int i=0; i < n; i++)//takes a copy of the tables before sending messages in this iteration starts
            routing_table.push_back(nd[i]->getTable());

        for (int i = 0; i < n; i++)//sends messages to all other nodes
            nd[i]->sendMsg();

        for(int i=0; i < n; i++)// check for convergence
        {
            if((nd[i]->getTable()).tbl.size()!=routing_table[i].tbl.size() )
            {
                flag=false;
                break;
            }
            for(int j=0; j < (nd[i]->getTable()).tbl.size(); j++)//compares the current and previous table
            {
                if((routing_table[i].tbl[j].nexthop == (nd[i]->getTable()).tbl[j].nexthop)&& (routing_table[i].tbl[j].ip_interface == (nd[i]->getTable()).tbl[j].ip_interface))
                {
                    continue;
                }
                else
                {
                    
                    flag=false;
                    break;

                }
            }
        }

    }

  /*Print routing table entries after routing algo converges */
  printRT(nd);
}


void RoutingNode::recvMsg(RouteMsg *msg) {
    int n=mytbl.tbl.size();
    int k=msg->mytbl->tbl.size();
    bool flag;
    for(int j=0;j<k;j++)
    {
        mytbl.tbl.push_back(msg->mytbl->tbl[j]);
        mytbl.tbl[mytbl.tbl.size()-1].cost++;
        mytbl.tbl[mytbl.tbl.size()-1].nexthop=msg->from;
        mytbl.tbl[mytbl.tbl.size()-1].ip_interface=msg->recvip;
        flag=false;
        for(int i=0;i<n;i++)
        {
            if(((mytbl.tbl[i].dstip) == (msg->mytbl->tbl[j]).dstip))
            {
                flag=true;//for duplicate entry
                if((mytbl.tbl[i].cost)>(msg->mytbl->tbl[j].cost+1))  //take shorter path
                {
                   // if(!isMyInterface(msg->mytbl->tbl[j].nexthop))//remove cycle within node,
                    //{
                        mytbl.tbl[i].nexthop=msg->from;
                        mytbl.tbl[i].ip_interface=msg->recvip;
                        mytbl.tbl[i].cost=msg->mytbl->tbl[j].cost+1;
              
                    //}
                    break;
                    
                }
            }
        }
        if(flag)// removes extra entry if required
        {
            mytbl.tbl.pop_back();
        }
    }

}




