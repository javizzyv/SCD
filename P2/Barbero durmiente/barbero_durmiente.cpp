#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;
int cliente_actual = -1;


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


void cortarPeloACliente()
{
   cout << "El barbero está pelando al cliente " << cliente_actual << endl;
   this_thread::sleep_for(chrono::milliseconds(aleatorio<350,500>()));
   cout << "El barbero ha acabado de pelar al cliente" << endl; 
}


void EsperarFueraBarberia(int cliente)
{
   cout << "\nEl cliente " << cliente << " ha salido fuera." << endl;
   this_thread::sleep_for(chrono::milliseconds(aleatorio<100,200>()));
   cout << "\nEl cliente " << cliente << " ha vuelto a entrar." << endl;

}

class Barberia : public HoareMonitor
{
    private:
        CondVar cola_barbero, cola_clientes, cola_silla;
    public:
        Barberia();
        int cortarPelo(int cliente);
        void siguienteCliente();
        void finCliente();
};


Barberia::Barberia()
{
    cola_barbero = newCondVar();
    cola_clientes = newCondVar();
    cola_silla = newCondVar();
}


int Barberia::cortarPelo(int cliente)
{
    while(!cola_silla.empty())
        cola_clientes.wait();
    if(!cola_barbero.empty())
        cout << "\nEl cliente " << cliente << " despierta al barbero" << endl;
    cout << "\nEl cliente " << cliente << " se sienta en la silla" << endl;

    cliente_actual = cliente;
    cola_barbero.signal();
    cola_silla.wait();
}


void Barberia::siguienteCliente()
{
    if(cola_clientes.empty() && cola_silla.empty()){
        cout << "\nEl barbero está durmiendo..." << endl;
        cola_barbero.wait();
    }
    cola_clientes.signal();
}


void Barberia::finCliente()
{
    cola_silla.signal();
}


void funcion_hebra_barbero(MRef<Barberia> monitor)
{
    while(true){
        monitor->siguienteCliente();
        cortarPeloACliente();
        monitor->finCliente();
    }
}


void funcion_hebra_cliente(MRef<Barberia> monitor, int cliente)
{
    while(true){
        monitor->cortarPelo(cliente);
        EsperarFueraBarberia(cliente);
    }
}


int main(){

    int num_clientes = 4;
    MRef<Barberia> monitor = Create<Barberia>();
    
    thread hebra_barbero(funcion_hebra_barbero, monitor),
        hebras_cliente[num_clientes];

    for(int i = 0; i < num_clientes; i++)
        hebras_cliente[i] = thread(funcion_hebra_cliente, monitor, i);
    for(int i = 0; i < num_clientes; i++)
        hebras_cliente[i].join();

    hebra_barbero.join();
}
