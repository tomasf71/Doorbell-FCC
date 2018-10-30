# Doorbell-FCC
Zvonek od přepravců je připojen na vstup D6 modulu WeMOS D1 (ESP8255) umístěném ve staré montáži na zdi. Napájen je z trafa na zdi u zadních vchodových dveří. Tento modul má bzučák, tlačítko a displej. Pomocí tlačítka lze přepínat kde bude zvonit bzučák při stlačení tlačítka u vchodu z parkoviště. To na koho je zvonek přepojen je signalizováno na LCD displeji. Zároveň je na LCD zobrazen nápis on-line jako příznak toho, že dotyčná osoba na kterou je přesměrováno má zařízení zapojené.

Je možno přesměrovat na tyto osoby:

Sklad - vyzvání pouze na zvoncích přiřazených skladu
Obchod - vyzvání u Dáši Ryskové + na zvoncích přiřazených skladu
Radka - vyzvání u Radky Helvichové + na zvoncích přiřazených skladu
ALL - vyzvání u všech
Jednotlivé krabičky jsou napájené pomocí USB portu a jsou pomocí WiFi připojené do sítě. To že na danou krabičku je přesměrováno zvonění, je signalizováno krátkým pípnutím a rozsvícením LED na krabičce.

Technická stránka: Všechny krabičky jsou připojené pomocí WiFi do sítě. Informace mezi sebou si předávají pomocí MQTT protokolu, přes MQTT server, který běží na našem NAS (Synology). Předávají si mezi sebou tyto zprávy:

Zmáčknutí zvonku u dveří: topic /fcc/doorbell hodnota = TRUE
Přepnutí na jiný zvonek: topic /fcc/listener hodnota = název na koho je zvonek přepnut
Informace že krabička nabootovala: topic /fcc/alive hodnota = název kdo nabootoval
Informace že krabička je online: topic /fcc/online hodnota = název kdo je online (posílá v případě, že je na něj přepnuto)
K tomu aby vše fungovalo, musí být v provozu hlavní krabička s displejem, fungovat WiFi a musí být spuštěný NAS (Synology) s nainstalovaným MQTT serverem(Mosquito). IP adresa NAS musí být nastavena na IP: 192.168.2.39

Zdrojový kód mají všechny krabičky stejný. Liší se pouze v definici jakou funkci konkrétní krabička bude mít. Definice ve zdrojovém kódu je zde: #define nazev „ Sklad“ tzn. role tohoto zařízení je Sklad

Pokud se nahraje program na nový modul [[https://wiki.wemos.cc/products:d1:d1_mini|WeMOS D1 (ESP8255)]] pak při prvním spuštění se aktivuje Wifi do režimu AP. Pokud se připojíte k tomuto AP máte možnost nakonfigurovat připojení k WiFi. Po uložení a restartu si již modul bude připojovat do na uloženou WiFi. V případě že WiFi je nedostupná, pak se opět aktivuje mód AP.

