#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>
using namespace std;

struct Pokemon {
    int numero;
    string nombre;
    string tipo1;
    string tipo2; // queda vacio si el pokemon tiene un solo tipo
    int total;
    int hp;
    int ataque;
    int defensa;
    int ataqueEsp;
    int defensaEsp;
    int velocidad;
    int generacion;
    bool legendario;
};

// Devuelve el texto en minusculas, para comparar sin importar mayusculas
string aMinusculas(string texto) {
    for (int i = 0; i < (int)texto.length(); i++) {
        texto[i] = tolower((unsigned char)texto[i]);
    }
    return texto;
}

// Convierte una linea del CSV en un Pokemon.
// getline con ',' lee hasta la siguiente coma; si el campo esta vacio
// (como el tipo 2 de Charmander) simplemente queda ""
Pokemon convertirLinea(string linea) {
    Pokemon p;
    stringstream lector(linea); // permite leer la linea por partes, como un archivo
    string campo;

    getline(lector, campo, ','); p.numero = stoi(campo);
    getline(lector, p.nombre, ',');
    getline(lector, p.tipo1, ',');
    getline(lector, p.tipo2, ',');
    getline(lector, campo, ','); p.total = stoi(campo);
    getline(lector, campo, ','); p.hp = stoi(campo);
    getline(lector, campo, ','); p.ataque = stoi(campo);
    getline(lector, campo, ','); p.defensa = stoi(campo);
    getline(lector, campo, ','); p.ataqueEsp = stoi(campo);
    getline(lector, campo, ','); p.defensaEsp = stoi(campo);
    getline(lector, campo, ','); p.velocidad = stoi(campo);
    getline(lector, campo, ','); p.generacion = stoi(campo);
    getline(lector, campo);
    p.legendario = (campo.find("True") == 0);
    return p;
}

// Lee el CSV completo y devuelve un arreglo dinamico con todos los pokemon.
// En *cantidad queda cuantos se leyeron. Si el archivo no abre, devuelve nullptr
Pokemon* cargarPokemon(string nombreArchivo, int* cantidad) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        *cantidad = 0;
        return nullptr;
    }

    string linea;
    int n = 0;

    // primera pasada: contar las lineas, sin el encabezado
    getline(archivo, linea);
    while (getline(archivo, linea)) {
        if (!linea.empty()) n++;
    }

    Pokemon* lista = new Pokemon[n];

    // segunda pasada: volver al inicio y llenar el arreglo
    archivo.clear();
    archivo.seekg(0);
    getline(archivo, linea); // encabezado otra vez
    int i = 0;
    while (i < n && getline(archivo, linea)) {
        if (linea.empty()) continue;
        lista[i] = convertirLinea(linea);
        i++;
    }

    archivo.close();
    *cantidad = n;
    return lista;
}

// Promedio de las 6 estadisticas de combate.
// Se divide entre 6.0 para que el resultado tenga decimales
float calcularPromedio(Pokemon* p) {
    int suma = p->hp + p->ataque + p->defensa + p->ataqueEsp + p->defensaEsp + p->velocidad;
    return suma / 6.0;
}

// Busca por numero de pokedex. Devuelve su direccion o nullptr
Pokemon* buscarPorNumero(Pokemon* lista, int cantidad, int numero) {
    for (int i = 0; i < cantidad; i++) {
        if (lista[i].numero == numero) {
            return &lista[i];
        }
    }
    return nullptr;
}

// Busca por nombre exacto, sin importar mayusculas
Pokemon* buscarPorNombreExacto(Pokemon* lista, int cantidad, string nombre) {
    string buscado = aMinusculas(nombre);
    for (int i = 0; i < cantidad; i++) {
        if (aMinusculas(lista[i].nombre) == buscado) {
            return &lista[i];
        }
    }
    return nullptr;
}

// Devuelve los pokemon cuyo nombre contiene el texto ("char" -> Charmander, Charmeleon, Charizard).
// Quien llama libera el arreglo con delete[]
Pokemon** buscarPorNombreParcial(Pokemon* lista, int cantidad, string texto, int* encontrados) {
    string buscado = aMinusculas(texto);

    int n = 0;
    for (int i = 0; i < cantidad; i++) {
        if (aMinusculas(lista[i].nombre).find(buscado) != string::npos) n++;
    }

    Pokemon** resultado = new Pokemon*[n];
    int pos = 0;
    for (int i = 0; i < cantidad; i++) {
        if (aMinusculas(lista[i].nombre).find(buscado) != string::npos) {
            resultado[pos] = &lista[i];
            pos++;
        }
    }

    *encontrados = n;
    return resultado;
}

// Devuelve los pokemon que tienen ese tipo como tipo 1 o tipo 2.
// Quien llama libera el arreglo con delete[]
Pokemon** buscarPorTipo(Pokemon* lista, int cantidad, string tipo, int* encontrados) {
    string buscado = aMinusculas(tipo);

    int n = 0;
    for (int i = 0; i < cantidad; i++) {
        if (aMinusculas(lista[i].tipo1) == buscado || aMinusculas(lista[i].tipo2) == buscado) n++;
    }

    Pokemon** resultado = new Pokemon*[n];
    int pos = 0;
    for (int i = 0; i < cantidad; i++) {
        if (aMinusculas(lista[i].tipo1) == buscado || aMinusculas(lista[i].tipo2) == buscado) {
            resultado[pos] = &lista[i];
            pos++;
        }
    }

    *encontrados = n;
    return resultado;
}

// Arma la lista de tipos distintos que aparecen en el archivo, sin repetir.
// Quien llama libera el arreglo con delete[]
string* obtenerTipos(Pokemon* lista, int cantidad, int* totalTipos) {
    // como mucho hay 2 tipos por pokemon, asi que ese espacio siempre alcanza
    string* tipos = new string[cantidad * 2];
    int n = 0;

    for (int i = 0; i < cantidad; i++) {
        string candidatos[2] = { lista[i].tipo1, lista[i].tipo2 };
        for (int c = 0; c < 2; c++) {
            if (candidatos[c].empty()) continue;
            bool repetido = false;
            for (int t = 0; t < n; t++) {
                if (tipos[t] == candidatos[c]) {
                    repetido = true;
                    break;
                }
            }
            if (!repetido) {
                tipos[n] = candidatos[c];
                n++;
            }
        }
    }

    *totalTipos = n;
    return tipos;
}

// Devuelve un arreglo de punteros ordenado por promedio, de mayor a menor.
// Se ordenan los punteros, asi la lista original queda igual.
// Quien llama libera el arreglo con delete[]
Pokemon** ordenarPorPromedio(Pokemon* lista, int cantidad) {
    Pokemon** orden = new Pokemon*[cantidad];
    for (int i = 0; i < cantidad; i++) {
        orden[i] = &lista[i];
    }

    // burbuja: en cada vuelta el de menor promedio baja hasta el final
    for (int i = 0; i < cantidad - 1; i++) {
        for (int j = 0; j < cantidad - 1 - i; j++) {
            if (calcularPromedio(orden[j]) < calcularPromedio(orden[j + 1])) {
                Pokemon* aux = orden[j];
                orden[j] = orden[j + 1];
                orden[j + 1] = aux;
            }
        }
    }
    return orden;
}

// Muestra toda la informacion de un pokemon
void mostrarPokemon(Pokemon* p) {
    cout << "\n#" << p->numero << " " << p->nombre << endl;
    cout << "  Tipo: " << p->tipo1;
    if (!p->tipo2.empty()) cout << " / " << p->tipo2;
    cout << endl;
    cout << "  HP: " << p->hp
         << "  Ataque: " << p->ataque
         << "  Defensa: " << p->defensa << endl;
    cout << "  Ataque Esp.: " << p->ataqueEsp
         << " Defensa Esp.: " << p->defensaEsp
         << " Velocidad: " << p->velocidad << endl;
    cout << "  Total: " << p->total
         << " Promedio: " << fixed << setprecision(2) << calcularPromedio(p) << endl;
    cout << "  Generacion: " << p->generacion
         << " Legendario: " << (p->legendario ? "Si" : "No") << endl;
}

// Muestra una lista corta: numero, nombre y tipos
void mostrarLista(Pokemon** lista, int cantidad) {
    if (cantidad == 0) {
        cout << "No se encontro ningun pokemon." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        cout << "  #" << lista[i]->numero << " " << lista[i]->nombre << " (" << lista[i]->tipo1;
        if (!lista[i]->tipo2.empty()) cout << "/" << lista[i]->tipo2;
        cout << ")" << endl;
    }
    cout << "Total: " << cantidad << endl;
}

// Lee un numero entero. Si escriben letras, lo vuelve a pedir
int leerEntero(string mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        string linea;
        if (!getline(cin, linea)) return -1; // se acabo la entrada
        stringstream lectorNumero(linea);
        if (lectorNumero >> valor) return valor;
        cout << "Eso no es un numero, intenta otra vez." << endl;
    }
}

// Lee una linea de texto
string leerTexto(string mensaje) {
    cout << mensaje;
    string texto;
    getline(cin, texto);
    return texto;
}

// Espera a que el usuario presione Enter, para que alcance a leer el resultado
// antes de que vuelva a salir el menu
void pausa() {
    cout << "\nPresiona Enter para continuar...";
    string basura;
    getline(cin, basura);
}

void mostrarMenu() {
    cout << "\n=== POKEDEX ===" << endl;
    cout << "1. Ver info por numero de pokedex" << endl;
    cout << "2. Buscar pokemon por nombre" << endl;
    cout << "3. Ver pokemon de un tipo" << endl;
    cout << "4. Ver info por nombre" << endl;
    cout << "5. Top 3 mas fuertes y mas debiles" << endl;
    cout << "0. Salir" << endl;
}

int main() {
    int cantidad;
    Pokemon* lista = cargarPokemon("pokemon_150_registros.csv", &cantidad);

    if (lista == nullptr) {
        cout << "No se pudo abrir pokemon_150_registros.csv." << endl;
        cout << "Revisa que este en la misma carpeta desde donde ejecutas el programa." << endl;
        return 0;
    }
    cout << "Se cargaron " << cantidad << " pokemon." << endl;

    int opcion;
    do {
        mostrarMenu();
        opcion = leerEntero("Elige una opcion: ");

        if (opcion == 1) {
            int numero = leerEntero("Numero de pokedex: ");
            Pokemon* p = buscarPorNumero(lista, cantidad, numero);
            if (p == nullptr) cout << "No existe un pokemon con el numero " << numero << "." << endl;
            else mostrarPokemon(p);

        } else if (opcion == 2) {
            string texto = leerTexto("Nombre o parte del nombre: ");
            int encontrados;
            Pokemon** resultado = buscarPorNombreParcial(lista, cantidad, texto, &encontrados);
            mostrarLista(resultado, encontrados);
            delete[] resultado; // solo la lista de punteros

        } else if (opcion == 3) {
            int totalTipos;
            string* tipos = obtenerTipos(lista, cantidad, &totalTipos);
            cout << "\nTipos disponibles:" << endl;
            for (int i = 0; i < totalTipos; i++) {
                cout << "  " << (i + 1) << ". " << tipos[i] << endl;
            }
            int eleccion = leerEntero("Elige un tipo: ");
            if (eleccion < 1 || eleccion > totalTipos) {
                cout << "Opcion de tipo invalida." << endl;
            } else {
                int encontrados;
                Pokemon** resultado = buscarPorTipo(lista, cantidad, tipos[eleccion - 1], &encontrados);
                cout << "\nPokemon de tipo " << tipos[eleccion - 1] << ":" << endl;
                mostrarLista(resultado, encontrados);
                delete[] resultado;
            }
            delete[] tipos;

        } else if (opcion == 4) {
            string nombre = leerTexto("Nombre exacto: ");
            Pokemon* p = buscarPorNombreExacto(lista, cantidad, nombre);
            if (p == nullptr) cout << "No existe un pokemon llamado '" << nombre << "'." << endl;
            else mostrarPokemon(p);

        } else if (opcion == 5) {
            Pokemon** orden = ordenarPorPromedio(lista, cantidad);
            cout << fixed << setprecision(2);
            cout << "\nTop 3 mas fuertes (promedio de estadisticas):" << endl;
            for (int i = 0; i < 3 && i < cantidad; i++) {
                cout << "  " << (i + 1) << ". " << orden[i]->nombre
                     << " - " << calcularPromedio(orden[i]) << endl;
            }
            cout << "\nTop 3 mas debiles:" << endl;
            for (int i = 0; i < 3 && i < cantidad; i++) {
                Pokemon* p = orden[cantidad - 1 - i];
                cout << "  " << (i + 1) << ". " << p->nombre
                     << " - " << calcularPromedio(p) << endl;
            }
            delete[] orden;

        } else if (opcion == 0 || opcion == -1) {
            cout << "Hasta luego." << endl;
            opcion = 0;

        } else {
            cout << "Opcion invalida." << endl;
        }

        if (opcion != 0) pausa();
    } while (opcion != 0);

    delete[] lista;
    return 0;
}
