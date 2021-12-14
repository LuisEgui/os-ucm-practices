#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define N_PARADAS 5		// número de paradas de la ruta
#define EN_RUTA 0		// autobús en ruta
#define EN_PARADA 1		// autobús en la parada
#define MAX_USUARIOS 40		// capacidad del autobús
#define USUARIOS 10		// numero de usuarios

int estado = EN_RUTA;		// estado inicial
int parada_actual = 0;		// parada en la que se encuentra el autobus
int n_ocupantes = 0;		// ocupantes que tiene el autobús

/* Personas que desean subir en cada parada */
int esperando_subir[N_PARADAS];	//= {0,0,...0};

/* Personas que desean bajar en cada parada */
int esperando_bajar[N_PARADAS];	//= {0,0,...0};

/* Otras definiciones globales (comunicación y sincronización) **/
pthread_mutex_t mutex;
pthread_cond_t busReady;
pthread_cond_t userWaiting;

/* Usuarios en el simulador */
pthread_t users[MAX_USUARIOS];
pthread_t bus;

/**
 * @brief Indica si el autobús está en condición de salir de la parada actual
 *
 * @return int
**/
int condicion_salir()
{
	return (esperando_subir[parada_actual] == 0 &&
		esperando_bajar[parada_actual] == 0);
}

/**
 * @brief Indica si el autobús está en condición de esperar en la parada actual
 *
 * @return int
**/
int condicion_espera()
{
	return (esperando_subir[parada_actual] > 0 ||
		esperando_bajar[parada_actual] > 0);
}

/**
 * @brief Ajusta el estado de la parada y bloquea al autobús hasta que no haya pasajeros que
 * quieran bajar y/o subir a la parada actual. Después se pone en marcha
 *
 * @return void
 **/
void Autobus_En_Parada()
{
	pthread_mutex_lock(&mutex);

	fprintf(stdout,
		"Bus at stop number [%d], actual number of passangers: %d\n",
		parada_actual, n_ocupantes);
	estado = EN_PARADA;

	while (condicion_espera() && n_ocupantes < MAX_USUARIOS) {
		pthread_cond_broadcast(&userWaiting);
		pthread_cond_wait(&busReady, &mutex);
	}

	fprintf(stdout,
		"Bus leaving stop number [%d], actual number of passangers: %d\n",
		parada_actual, n_ocupantes);

	pthread_mutex_unlock(&mutex);
}

/**
 * @brief Establece un retardo aleatorio que simula el trayecto 
 * y actualiza el número de parada.
 *
 * @return void
 **/
void Conducir_Hasta_Siguiente_Parada()
{
	pthread_mutex_lock(&mutex);

	fprintf(stdout, "Bus driving to next stop...\n");
	estado = EN_RUTA;
    pthread_mutex_unlock(&mutex);

	sleep(random() % 2);

    pthread_mutex_lock(&mutex);
	parada_actual = (parada_actual + 1) % N_PARADAS;
	pthread_mutex_unlock(&mutex);
}

/**
 * @brief El usuario indicará que quiere subir en la parada
 * 'origen', esparará a que el autobús se pare en dicha parada
 * y subirá. El id_usuario puede utilizarse para proporcionar
 * información de depuración.
 *
 * @param id_usuario identificador del usuario que quiere subir
 * @param origen Parada en la que el usuario está esperando para subir
 * @return void
 **/
void Subir_Autobus(int id_usuario, int origen)
{
	pthread_mutex_lock(&mutex);

	fprintf(stdout,
		"User: [%d] waiting at origin [%d] to get on the bus...\n",
		id_usuario, origen);
	esperando_subir[origen]++;

	while (parada_actual != origen || estado == EN_RUTA || n_ocupantes > MAX_USUARIOS)
		pthread_cond_wait(&userWaiting, &mutex);

	n_ocupantes++;
	esperando_subir[origen]--;
	fprintf(stdout, "User: [%d] just got on the bus...\n", id_usuario);

	if (condicion_salir()) {
		fprintf(stdout, "No more people getting on/off the bus.\n");
		pthread_cond_signal(&busReady);
	}

	pthread_mutex_unlock(&mutex);
}

/**
 * @brief El usuario indicará que quiere subir en la parada
 * 'destino', esparará a que el autobús se pare en dicha parada
 * y bajará. El id_usuario puede utilizarse para proporcionar
 * información de depuración.
 *
 * @param id_usuario identificador del usuario que quiere bajar
 * @param destino Parada en la que el bajará
 * @return void
 **/
void Bajar_Autobus(int id_usuario, int destino)
{
	pthread_mutex_lock(&mutex);

	esperando_bajar[destino]++;
	fprintf(stdout, "User: [%d] waiting to get off the bus...\n",
		id_usuario);

	while (parada_actual != destino || estado == EN_RUTA)
		pthread_cond_wait(&userWaiting, &mutex);

	n_ocupantes--;
	esperando_bajar[destino]--;
	fprintf(stdout, "User: [%d] just got off the bus...\n", id_usuario);

	if (condicion_salir()) {
		fprintf(stdout, "No more people getting on/off the bus.\n");
		pthread_cond_signal(&busReady);
	}

	pthread_mutex_unlock(&mutex);
}

/**
 * @brief Hilo del autobús. Simula el comportamiento de un
 * autobús: espera en la parada, conduce hasta la siguiente.
 *
 * @param arg N/A.
 * @return void
**/
void *thread_autobus(void *arg)
{
	/* El autobús circula constantemente */
	while (1) {
		/** Esperar a que los viajeros suban y bajen */
		Autobus_En_Parada();
		/** Conducir hasta la siguiente parada */
		Conducir_Hasta_Siguiente_Parada();
	}
}

/**
 * @brief Simula la acción del usuario: subir y bajar del autobús.
 *
 * @param id_usuario Identificador del usuario
 * @param origen Parada de origen
 * @param destino Parada de destino
 * @return void
 **/
void Usuario(int id_usuario, int origen, int destino)
{
	fprintf(stdout, "CREATION-INFO: UID: %d -- From: %d to %d\n",
		id_usuario, origen, destino);
	Subir_Autobus(id_usuario, origen);
	Bajar_Autobus(id_usuario, destino);
}

/**
 * @brief Hilo de usuario, identificado por un 'id_usuario',
 * un 'origen' y un 'destino' determinados aleatoriamente.
 *
 * @param arg 'id' del usuario
 * @return void
**/
void *thread_usuario(void *arg)
{
	int id_usuario = (void *)(size_t)arg;
	int a, b;

	while (1) {
		a = rand() % N_PARADAS;
		do {
			b = rand() % N_PARADAS;
		} while (a == b);

		Usuario(id_usuario, a, b);
	}
}

void signal_handler(int sig)
{
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&busReady);
	pthread_cond_destroy(&userWaiting);

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	unsigned long i;

	signal(SIGKILL, signal_handler);
	signal(SIGINT, signal_handler);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&busReady, NULL);
	pthread_cond_init(&userWaiting, NULL);

	for (i = 0; i < N_PARADAS; ++i) {
		esperando_bajar[i] = 0;
		esperando_subir[i] = 0;
	}

	pthread_create(&bus, NULL, thread_autobus, NULL);

	for (i = 0; i < USUARIOS; ++i)
		pthread_create(&users[i], NULL, thread_usuario, (void *)i);

	pthread_join(bus, NULL);

	for (i = 0; i < USUARIOS; ++i)
		pthread_join(users[i], NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&busReady);
	pthread_cond_destroy(&userWaiting);

	exit(EXIT_SUCCESS);
}
