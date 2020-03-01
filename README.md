## ISA Projekt: Čtečka novinek ve formátu Atom s podporou TLS
### Autor: Jan Vávra (xvavra20)

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

#### Rozšíření/Omezení
+ Program podporuje vybrané elementy z Dublin Core (creator, modified, title)
+ Program umí zpracovat elementy z jiných formátů (např. elementy Atom v RSS feedu, elementy RSS 1.0 v RSS 2.0 apod.)
+ Program umí zpracovat URL adresy ve *feedfile* oddělené koncem řádku (\n) nebo oddělené **mezerou**

#### Popis varianty:
Napište program feedreader, který bude vypisovat informace uvedené ve stažených zdrojích (feed) ve formátu Atom a RSS. Program po spuštění stáhne zadané zdroje a na standardní výstup vypíše informace požadované uživatelem (např. názvy článků).

Při vytváření programu je povoleno použít hlavičkové soubory pro práci se sokety a další obvyklé funkce používané v síťovém prostředí (jako je netinet/*, sys/*, arpa/* apod.), knihovnu pro práci s vlákny (pthread), signály, časem, stejně jako standardní knihovnu jazyka C (varianty ISO/ANSI i POSIX), C++ a STL. Pro práci s daty ve formátu XML je doporučená knihovna libxml2, případně jiné knihovny dostupné na serveru merlin a eva. Pro práci s TLS je doporučená knihovna openssl, případně jiné knihovny dostupné na serveru merlin a eva. Pro práci s XML a TLS můžete použít i jiné knihovny, musí však být jasně označeny, že jde o převzatý kód a musí být součástí odevzdávaného archivu. Jiné knihovny nejsou povoleny.

#### Spuštění aplikace
Použití: 
```
feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]
```

Pořadí parametrů je libovolné. Popis parametrů:

- Povinně je uveden buď URL požadovaného zdroje (přičemž podporovaná schémata jsou http a https), nebo parametr -f s dodatečným parametrem určujícího umístění souboru feedfile. Soubor feedfile je textový soubor, kde je na každém řádku uvedena jedna adresa zdroje ve formátu Atom, či RSS. Prázdné řádky v souboru feedfile ignorujte. Řádky začínající znakem '#' v souboru feedfile ignorujte, jsou to komentáře.  Soubor feedfile je obvyklý Unixový textový soubor, tzn. poslední znak na každém řádků je LF. Z toho také vyplývá, že poslední znak v souboru je LF.
- Volitelný parametr -c definuje soubor <certfile> s certifikáty, který se použije pro ověření platnosti certifikátu SSL/TLS předloženého serverem.
- Volitelný parametr -C určuje adresář <certaddr>, ve kterém se mají vyhledávat certifikáty, které se použijí pro ověření platnosti certifikátu SSL/TLS předloženého serverem.
- Pokud není uveden parametr -c ani -C, pak použijte úložiště certifikátů získané funkcí SSL_CTX_set_default_verify_paths().
- Při spuštění s parametrem -T se pro každý záznam zobrazí navíc informace o čase změny záznamu, či vytvoření záznamu (je-li ve staženém souboru obsaženo).
- Při spuštění s parametrem -a se pro každý záznam zobrazí jméno autora, či jeho e-mailová adresa (je-li ve staženém souboru obsaženo).
- Při spuštění s parametrem -u se pro každý záznam zobrazí asociované URL (je-li ve staženém souboru obsaženo).

#### Výstup aplikace
Pro každý zdroj vypisujte informace v následujícím tvaru:
- 1. řádek: Název zdroje uvozený znaky "*** " a ukončený znaky " ***", např. "*** FIT VUT v Brně ***".
- Na dalších řádcích budou uvedeny jednotlivé záznamy. V základním tvaru na každém řádku titulek jednotlivých záznamů, např. "10.5.2017 Erasmus+ výjezdy a zahraniční stáže/praxe".
- V případě použití parametrů -T, -a a -u budou na dalších řádcích pod zdrojem uvedeny další dodatečné informace uvozeny řetězcem "Aktualizace: ", "Autor: ", respektive "URL: ". Při použití dodatečných atributů budou od sebe jednotlivé záznamy odděleny jedním prázdným řádkem.
- V případě použití parametru -f budou od sebe jednotlivé zdroje odděleny jedním volným řádkem.
