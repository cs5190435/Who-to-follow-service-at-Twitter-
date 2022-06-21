#include <string>
#include <mpi.h>
#include <assert.h>
#include <fstream>
#include <chrono>
#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include "randomizer.hpp"

//Notice how the randomizer is being used in this dummy function

struct List{
    std::vector<int> l;
    int size= 0;
};


bool cmp(std::pair<int,int> a, std::pair<int,int> b){
    if(a.second == b.second){return a.first > b.first;}
    else{return a.second < b.second;}
}

int SWAP_INT32(int x){
    //char *c = (std::to_string(x& 0x000000FF) + std::to_string((x & 0x0000FF00)) + std::to_string((x & 0x00FF0000))+ std::to_string((x & 0xFF000000))) ;
    //return ((x & 0x000000FF << 24) |(((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8) |(x >> 24);
    return (x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24);

}
int print_random(int tid, int fin, std::vector<List> AdjList, int num_nodes, Randomizer r, int walks, int steps, int rec){
    std::ofstream wf("output.dat",std::ios::out | std::ios::binary);
    //std::ifstream rf("result.dat", std::ios::out | std::ios::binary);
    if(!wf){
        std::cout << "Unable to open the file" << std::endl;
        return -1;
    }
    for(int i= tid*fin; i< std::min(fin*(tid+1),num_nodes); i++){
        List req = AdjList[i];
        //int influence[num_nodes] = {0};
        std::vector<int> influence(num_nodes,0);
        for(int j= 0; j< req.size; j++){
            //if(i==8716){std::cout << req.l.at(j) << " ";}
            for(int k= 0; k< walks; k++){
                int current = req.l.at(j);
                for(int m=0; m< steps; m++){
                    if(AdjList.at(current).size>0){
                        int nextstep = r.get_random_value(i);
                        if(nextstep < 0){current = req.l.at(j);}
                        else{
                            int ind = nextstep%(AdjList.at(current).size);
                            current = AdjList.at(current).l.at(ind);
                        }
                    }
                    else{current = req.l.at(j);}
                    influence[current] +=1;
                }
            }      
        }
        //if(i==8716){std::cout << " " << std::endl;}
        influence[i] =0;
        for(int j=0; j< req.size;j++){
            influence[req.l.at(j)]= 0;
        }
        //if(i==8714){std::cout << influence[8603] << std::endl;}
        std::vector<std::pair<int,int>> vp;
        for(int x= 0; x<num_nodes; x++){vp.push_back(std::make_pair(x,influence[x]));}
        std::sort(vp.begin(), vp.end(),cmp);
        
        int modi = SWAP_INT32(req.size);
        //char buf[4];
        wf.seekp(i*(4+8*rec));
        wf.write((char *)&modi,4);
        //rf.read((char *)&buf,4);
        //int dum = (int)buf[3] | (int)buf[2]<<8 | (int)buf[1]<<16 | (int)buf[0]<<24;

        if(req.size ==0){
            for(int x = 0; x< rec; x++){
                char bufer[8] = {'N','U','L','L','N','U','L','L'};
                wf.write((char *)&bufer, 8);
            }
        }
        
        else{
            /*int x= 0;
            int z= 0;
            std::vector<std::pair<int,int>> final;
            while(z < rec){
                int y = vp[num_nodes-1-x].first;
                if(doesnotbelong(y,req.l,req.size) && y!=i){
                    final.push_back(std::make_pair(y,vp[num_nodes-1-x].second));
                    z +=1;
                }
                x +=1;
            }*/
            for(int x=0;x<rec; x++){
                //int third = rf.read((char *)&buf,4);
                //int fourth = rf.read((char *)&buf,4);
                if(vp.at(num_nodes-1-x).second != 0){
                    
                    int second = SWAP_INT32(vp.at(num_nodes-x-1).second);
                    int first = SWAP_INT32(vp.at(num_nodes-1-x).first);
                    //if(i==8714){std::cout << vp.at(num_nodes-1-x).first << ":" << vp.at(num_nodes-1-x).second << " ";}
                    wf.write((char *)&first,4);
                    wf.write((char *)&second,4);
                }
                else{
                    //if(i>39){std::cout << "NULL:NULL" << " ";}
                    char buff[8] = {'N','U','L','L','N','U','L','L'};
                    wf.write((char *)&buff,8);
                }
            }
        }
        std::cout << i << std::endl;

        //if(i>39){std::cout << " " << std::endl;}    std::cout << i << std::endl;

    }
    wf.close();
    /*if(output.dat == result.dat){
        std::cout << "Success" << std::endl;
    }*/
    return 0;
}

int main(int argc, char* argv[]){
    assert(argc > 8);
    std::string graph_file = argv[1];
    int num_nodes = std::stoi(argv[2]);
    int num_edges = std::stoi(argv[3]);
    float restart_prob = std::stof(argv[4]);
    int num_steps = std::stoi(argv[5]);
    int num_walks = std::stoi(argv[6]);
    int num_rec = std::stoi(argv[7]);
    int seed = std::stoi(argv[8]);

    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<List> AdjList;
    const char* c = graph_file.c_str();
    for(int i=0; i < num_nodes; i++){
        List req;
        AdjList.push_back(req);
    }
    //std::cout << AdjList.size() << std::endl;
    FILE * fp;
    unsigned char buffer[8];
    int temp;
    fp = fopen(c,"rb");
    //std::ofstream fw("testing.txt",std::ofstream::out);
    while(!feof(fp)){
        fread(&buffer,1,8,fp);
        int num = (int)buffer[3] | (int)buffer[2]<<8 | (int)buffer[1]<<16 | (int)buffer[0]<<24;
        int req = (int)buffer[7] | (int)buffer[6]<<8 | (int)buffer[5]<<16 | (int)buffer[4]<<24;
        AdjList[num].l.push_back(req);
        AdjList[num].size +=1;
        temp = num;
        //fw << num << " " << req << "\n";
    }
    std::cout << temp << std::endl;
    AdjList[temp].size -=1;
    AdjList[temp].l.pop_back();
    
    //Only one randomizer object should be used per MPI rank, and all should have same seed
    Randomizer random_generator(seed, num_nodes, restart_prob);
    int rank, size;

    //Starting MPI pipeline
    MPI_Init(NULL, NULL);
    
    // Extracting Rank and Processor Count
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //std::cout << sizeof("NULLNULL") << std::endl;
    
    int final;
    if(num_nodes%size == 0){
        final = num_nodes/size;
    }
    else{
        final = (num_nodes/size) +1;
    }
    print_random(rank, final, AdjList, num_nodes, random_generator,num_walks,num_steps,num_rec);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    double duration = (1e-6 * (std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)).count());
    std::cout << "Time taken : " << duration/1000 << std::endl;
    MPI_Finalize();
}