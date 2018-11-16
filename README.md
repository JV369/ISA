## ISA Projekt: Čtečka novinek ve formátu Atom s podporou TLS
###Autor: Jan Vávra (xvavra20)

#### Popis
Program umožňující přečíst RSS/Atom feedy zadané URL adresou.

#### Překlad
+ `make` - přeloží program a umístí ho do aktuálního adresáře
+ `make test` - přeloží program a spustí testy
+ `make clean` - smaže program feedreader z aktuálního adresáře

#### Spuštění 
```
./feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u] [-h]
```
Povinné:
+ `URL` - adresa feedu, který má program přečíst
+ `-f <feedfile>` - přepínač -f se specifikací cesty k souboru s URL odkazující na feedy

Volitelné:
+ `-c <certfile>` – přepínač -c se specifikací cesty k souboru s certifikátem, který má použít při připojení na
  URL
+ `-C <certaddr>` – přepínač -C se specifikací cesty k adresáři, který obsahuje
+ `-T` – přepínač -T, který zajistí vypsání času aktualizace položky, pokud je dostupný
+ `-a` – přepínač -T, který zajistí vypsání autora položky, pokud je dostupný
+ `-u` – přepínač -T, který zajistí vypsání URL položky, pokud je dostupný

#### Seznam souborů
+ main.c - hlavní vstup programu
+ queue.c - pomocná datová struktura fronta
+ argChecker.c - zpracovává argumenty na vstupu
+ connect.c - připojjuje se na URL a stáhne její obsah
+ feeder.c - zpracuje stažený obsah na výstup
+ test.sh - skript, který testuje program
+ test/ - soubory, které zpracovává test.sh (vstupy, očekávané výstupy, návratové kody)
+ test/pass/ - soubor testovacích vstupů a výstupů, kde program skončí úspěšně
+ test/fail/ - soubor testovacích vstupů a výstupů, kde program skončí chybou
+ manual.pdf - dokumentace

####Rozšíření/Omezení
+ Program podporuje vybrané elementy z Dublin Core (creator, modified, title)
+ Program umí zpracovat elementy z jiných formátů (např. elementy Atom v RSS feedu, elementy RSS 1.0 v RSS 2.0 apod.)
+ Program umí zpracovat URL adresy ve *feedfile* oddělené koncem řádku (\n) nebo oddělené **mezerou**
