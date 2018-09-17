#include <mpi.h>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

#define TAG 0

int main(int argc, char *argv[])
{
	int pocetCisel; // celkový pčoet čísel
    int processors; // počet procesorů
    int myId; // číslo procesoru
	int hotovo =0;	// stav algoritmu
	int sude = 0; // režim cyklu
	int sudyProcesor = 1; // sudý procesor
	int procesorPosila; 
	int procesorPrijima;
	double start, end; 
	
	std::vector<int> cisla; //seznam všech čísel
	std::vector<int> posloupnostVector; //posloupnost daného procesu
	int velikostPosloupnost; // velikost seznamu posloupnosti
	std::vector<int> posloupnostVectorPosilany; // vektor, který je posílaný
	int velikostPosloupnostPosilana;
    MPI_Status stat;

    //MPI INIT
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);

	 if (myId == 0) { // práce 1. řídícího procesoru
        int number;
        fstream fin;
        fin.open(argv[1], ios::in); //čtení vstupního souboru
        while (fin.good()){
            number = fin.get();
            if(fin.eof())
                break;
            cout << number;
			cout << " ";
			cisla.push_back(number);
			pocetCisel++;			
        }
		int podil = cisla.size()/processors; //nastavení poměru čísel a procesorů
		int zbytek = cisla.size()%processors; // zbytek poměru

		int prvek = 0;
		for (int i = 0; i < processors; i++){ // cyklus pro přiřazení a rozeslání čísel
		    if(zbytek > 0){ // zjištění velikosti pole
				velikostPosloupnost = podil+1;
			}else{
				velikostPosloupnost = podil;
			}
			posloupnostVector.clear();
			
			for (int u = 0; u < velikostPosloupnost; u++){ //Přiřazení čísel do posloupnosti
				posloupnostVector.push_back(cisla[prvek]);		
				prvek++;
			}
			if(zbytek>0){ //odebrání zbytku
				zbytek--;
			}
			MPI_Send(&velikostPosloupnost, 1, MPI_INT, i, TAG, MPI_COMM_WORLD); //zaslání velikosti vektoru
			MPI_Send(&posloupnostVector[0], velikostPosloupnost, MPI_INT, i, TAG, MPI_COMM_WORLD); //zaslání vektoru
		}
        fin.close();
        cout << endl;
    }
	MPI_Recv(&velikostPosloupnost, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); // Přijmutí posloupnosti
	posloupnostVector.resize(velikostPosloupnost); // nastavení velikosti vektoru
	MPI_Recv(&posloupnostVector[0], velikostPosloupnost, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //přijmutí vektoru
    MPI_Barrier(MPI_COMM_WORLD); //synchronizace procesů
    start = MPI_Wtime(); // začátek měření
	sort(posloupnostVector.begin(), posloupnostVector.end()); // sekvenční seřazení vektoru
	if ( myId % 2 == 0){ // nastavení, zda je procesor sudý/lichý
		sudyProcesor = 1;
	}else{
		sudyProcesor = 0;
	}
	sude = 1; // nastavení režimu
 	while (hotovo == 0) {		
		procesorPosila = 0;
		procesorPrijima = 0;
		posloupnostVectorPosilany = posloupnostVector; 
		velikostPosloupnostPosilana = velikostPosloupnost;
		if(sude == 1){//Stav sudý
			if(sudyProcesor == 1){ //procesor je sudý
				if(myId+1<processors){//Má další procesor
					procesorPrijima = 1;
				}else{//Nemá další procesor
				}
			}else{//procesor není sudý				
				procesorPosila = 1;
			}
		}else{//Stav lichý
			if(sudyProcesor == 0){//procesor je lichý
				if(myId+1<processors){//Má další procesor
					procesorPrijima = 1;
				}else{//Nemá další procesor
				}
			}else{//procesor není lichý
				if(myId != 0){//proces není 0
					procesorPosila = 1;
				}			
			}
		}
		if(procesorPosila == 1){ //poslání vektoru
			MPI_Send(&velikostPosloupnostPosilana, 1, MPI_INT, myId-1, TAG, MPI_COMM_WORLD);
			MPI_Send(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, myId-1, TAG, MPI_COMM_WORLD);
		}
		if(procesorPrijima == 1){ // přijmutí vektoru
			MPI_Recv(&velikostPosloupnostPosilana, 1, MPI_INT, myId+1, TAG, MPI_COMM_WORLD, &stat);
			posloupnostVectorPosilany.resize(velikostPosloupnostPosilana);
			MPI_Recv(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, myId+1, TAG, MPI_COMM_WORLD, &stat);	
			int velikostNovehoPosloupnost = velikostPosloupnost + velikostPosloupnostPosilana;

			std::vector<int> novePosloupnostVector;
			int indexL = 0; // index levého vektoru
			int indexR = 0; // index pravého vektoru
			int serazeno = 0;
			int leva;
			int prava;
			while (serazeno == 0){
				leva =0;
				prava=0;
				if(posloupnostVector[indexL]<=posloupnostVectorPosilany[indexR]){ // Zjištění, ve kterém vektoru je nižší hodnota
					if(indexL < velikostPosloupnost){
						leva = 1;
					}else{
						prava = 1;
					}					
				}else{
					if(indexR < velikostPosloupnostPosilana){
						prava = 1;
					}else{
						leva = 1;
					}					
				}
				if(leva == 1){ // přidání nejnižšího čísla, do nového vektoru
					novePosloupnostVector.push_back(posloupnostVector[indexL]);	
					indexL++;
				}
				if(prava == 1){
					novePosloupnostVector.push_back(posloupnostVectorPosilany[indexR]);	
					indexR++;
				}
				
				if(novePosloupnostVector.size()==velikostNovehoPosloupnost){
					serazeno = 1;
				}
			}
			posloupnostVector.clear(); // vymazání původního obsahu vektorů
			posloupnostVectorPosilany.clear();
			for(int i=0; i<novePosloupnostVector.size();i++){ //přiřazení seřazených hodnot do vektorů
				if(i<velikostPosloupnost){ // Přiřazení do 1. vektoru
					posloupnostVector.push_back(novePosloupnostVector[i]);
				}else{ // Přiřazení do 2. vektoru
					posloupnostVectorPosilany.push_back(novePosloupnostVector[i]);	
				}
			}
			MPI_Send(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, myId+1, TAG, MPI_COMM_WORLD); // Vrácení seřazeného vektoru
		}
		if(procesorPosila == 1){ // Přijmutí seřazeného vektoru			
		    posloupnostVector.clear();
			posloupnostVectorPosilany.clear();
			posloupnostVectorPosilany.resize(velikostPosloupnostPosilana);
			MPI_Recv(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, myId-1, TAG, MPI_COMM_WORLD, &stat);
			posloupnostVector = posloupnostVectorPosilany;
		}
		posloupnostVectorPosilany = posloupnostVector;
		velikostPosloupnostPosilana = posloupnostVectorPosilany.size();
		MPI_Send(&velikostPosloupnostPosilana, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD); //Zaslání vektorů do řídícího procesů
		MPI_Send(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, 0, TAG, MPI_COMM_WORLD);

		if(myId == 0){	// Řídící procesor zkoumá, zda je celá posloupnost seřazená 	
			std::vector<int> celaPosloupnost;
			int prvekPosloupnosti;
			for(int i =0; i < processors; i++){ // Přijmutí vektoru ze všech procesorů a přiřazení čísel do nového vektoru 
				posloupnostVectorPosilany.clear();
				MPI_Recv(&velikostPosloupnostPosilana, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
				posloupnostVectorPosilany.resize(velikostPosloupnostPosilana);
				MPI_Recv(&posloupnostVectorPosilany[0], velikostPosloupnostPosilana, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
				for(int u=0; u < posloupnostVectorPosilany.size(); u++){
					celaPosloupnost.push_back(posloupnostVectorPosilany[u]);
				}	
			}
			for(int i=0; i<celaPosloupnost.size();i++){	// Kontrola nového vektoru, zda je posloupnost seřazená
				if(i+1<celaPosloupnost.size()){				
					if(celaPosloupnost[i]>celaPosloupnost[i+1]) // Kontrola, zda následující prvek není menší, než aktuální
						i = celaPosloupnost.size()+1; // zrušení cyklu (posloupnost není seřazená)
				}
				if(i == celaPosloupnost.size()-1){ // Celá posloupnost je seřazená
					end = MPI_Wtime(); // ukončení měření času
					for(int i=0; i<celaPosloupnost.size();i++){ // výpis všech seřazených čísel
						std::cout << celaPosloupnost[i]<< std::endl;
					}
					hotovo =1; // zrušení cyklu (celá posloupnost je seřazená)
				}
			}
			if(sude ==1){ //Nastavení režimu pro další iteraci
				sude=0;
			}else if(sude ==0){
				sude =1;
			}
			for(int i=0; i<processors;i++){ // zaslání informace o stavu algoritmu dalším procesorům
				MPI_Send(&sude, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
				MPI_Send(&hotovo, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);				
			}			
		}
		MPI_Recv(&sude, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //přijmutí informace o stavu algoritmu 
		MPI_Recv(&hotovo, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
		MPI_Barrier(MPI_COMM_WORLD);
	}
  
    //K měření času
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (myId == 0) { // Výpis měření času
  //      cerr << end-start << endl;
    } 
	MPI_Finalize();
    return 0;
}
