#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int koltuk_sayisi = 2, bos_koltuk_sayisi = 2, musteri_sayisi = 0, sandalye_sayisi = 5, bos_sandalye_sayisi = 0, serve_client = 0, dolu_sandalye = 0, bekleme_suresi=0;
int* koltuk;

sem_t berberler_sem;  //Kullanilacak sem'ler
sem_t musteriler_sem;
sem_t mutex_sem;

void Berber(void* num)
{
    int berber_id = *(int*)num +1; //berber_id icerisine parametre olarak gelen veriyi atadik.
    int sonrakiMusteri, musteri_id;
    printf("%d. Numarali Berber Salona Girdi.\n",berber_id);
    while(1)    
    {
        if (bos_sandalye_sayisi==5)//Salonda Musteri Olup Olmadigini Kontrol Ediyoruz.
        {
            printf("*****%d. Numarali Berber Uyuyor.*****\n\n", berber_id);     //Berber Musteri Yoksa Uyuyor.
        }
        sem_wait(&berberler_sem);
        sem_wait(&mutex_sem);
        serve_client = (++serve_client) % sandalye_sayisi;  //Trasa Olacak Kisi Bekleyen Müsterilerden Birisi Olur.
        sonrakiMusteri = serve_client;
        musteri_id = koltuk[sonrakiMusteri];
        koltuk[sonrakiMusteri] = pthread_self();
        sem_post(&mutex_sem);   //Sandalyeye Erisim Acilir Ve Berber Trasa Baslar
        sem_post(&musteriler_sem);
		bos_koltuk_sayisi--;
        printf("%d. Numarali Berber %d. Numarali Musteriyi Trasa Basladi.\n\n", berber_id, musteri_id);   //Belirtilen Tras Suresinden Sonra Tras Biter.
        sleep(2);
        bos_koltuk_sayisi++;
        printf("%d. Numarali Berber %d. Numarali Musterinin Trasini Bitirdi.\n\n",berber_id, musteri_id);
    }

    pthread_exit(0);
}

void Musteri(void* num)  //Musteri Fonksiyonunda Parametre Olarak Musteri Numarasini Alir.
{
    int n = *(int*)num +1;
    int sandalyeDurumu, barber_identity;
    sem_wait(&mutex_sem);   //Koltuga Erisim Saglanamaz.
    printf("%d. Numarali Musteri Salona Geldi. \n", n);      //Gelen Musteri Ekrana Yazilir.
    if (bos_sandalye_sayisi > 0)
    {
        bos_sandalye_sayisi--;     //Sandalye Sayisini 1 Azaltýr.
		if(bos_koltuk_sayisi==0)
        printf("%d. Numarali Musteri Bekliyor.\n\n", n);
        dolu_sandalye = (++dolu_sandalye) % sandalye_sayisi;
        sandalyeDurumu = dolu_sandalye;
        koltuk[sandalyeDurumu] = n;     //Musteri'nin bos sandalyelerden birine oturmasina izin verilir
        sem_post(&mutex_sem);
        sem_post(&berberler_sem);  //Sandalyeye Erisim Kaldirilir Ve Uyuyan Berber Uyandirilir.
        sem_wait(&musteriler_sem);
        sem_wait(&mutex_sem);
        barber_identity = koltuk[sandalyeDurumu];    //Berber takes care of the musteri
        bos_sandalye_sayisi++;
        sem_post(&mutex_sem);
    }
    else
    {
        sem_post(&mutex_sem);
        printf("%d. Numarali Musteri Oturacak Yer Bulamadigi Icin Salonu Terk Etti.\n\n", n);  //Bos Sandalye Yoksa Salondan Ayrilir.
    }
    pthread_exit(0);
}

void Bekle()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 +1) + 50000);
}

int main (int argc, char** args)
{
    printf("\t\t*****************F-O-A Salon'a Hosgeldiniz*****************\n\n");
    printf("\t\t\tBerber\t\tKoltuk\t\tSandalye\n");
    printf("\t\t\t  2\t\t  2\t\t    5\n\n");
    printf("Musteri Sayisini Girin : ");   // Gelecek Olan Toplam Musteri Sayisini Giriyoruz.
    scanf("%d", &musteri_sayisi);
    while(1){
    	printf("Musteri Gelme Arasindaki Zaman Farki (0 , 1 veya 2) : ");   // Gelecek olan müsteriler arasýndaki zaman farkini ayarlamak icin
    	scanf("%d", &bekleme_suresi);
    	if(bekleme_suresi>=0 && bekleme_suresi<=2)
    	break;
	}
    bos_sandalye_sayisi = sandalye_sayisi;
    koltuk = (int*) malloc(sizeof(int) * sandalye_sayisi);   //Limitini Belirledigimiz Koltuk dizisini Olusturduk.
    pthread_t berber[koltuk_sayisi], musteri[musteri_sayisi];     //Berber ve Musteri Degiskenleri Olusturma.
    sem_init(&berberler_sem, 0, 0);
    sem_init(&musteriler_sem, 0, 0);
    sem_init(&mutex_sem,0, 1);
    printf("\nBerber salonu acti.\n\n");
	int i;
    for(i = 0; i<koltuk_sayisi; i++)
    {
        pthread_create(&berber[i], NULL, (void*)Berber, (void*)&i);     //Berber Dizileri Olusturma.
        sleep(1);
    }
    for(i = 0; i < musteri_sayisi; i++)
    {

        pthread_create(&musteri[i], NULL, (void*)Musteri, (void*)&i);  //Musteri Dizileri Olusturma.
        Bekle();
        sleep(bekleme_suresi);
    }
    for(i = 0; i < musteri_sayisi; i++)
    {
        pthread_join(musteri[i], NULL);
    }
    sleep(3);
    printf("\n Tum Traslar Bitti. Berberler Salonu Kapatti!\n\n");  // Berber Salonu Kapandi.
    return EXIT_SUCCESS;
}
