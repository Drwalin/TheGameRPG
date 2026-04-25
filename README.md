
# TheGameRPG

NOWA NAZWA

+ podzielić to repo na silnik/klient/serwer/gra

## Already Implemented

+ ECS
+ scripting
+ efficient spatial partitioning
+ efficient collision
+ serialization
+ events system
+ audio
+ input
+ very basic and primitive rendering
+ animations
+ particles

## Common Engine Elements

+ NIECH SIE W KOŃCU URUCHOMI POPRAWNIE
+ zmiana nazwy Realm -> Region (albo coś lepszego?)

- stworzenie nowego nazewnictwa i podziału:
  - Zone: grupa regionów (potencjalnie na jednym fizycznym serwerze),
      np. regiony z tym samym biomem
- prefabs for mobs and static objects to reduce memory and network usage

## Server Elements

+ baza danych poziomów (RocksDB) (delta update do db)
+ byle jakie AI do walki, chodzenie wprost do przeciwnika
+ budowanie i niszczenie świata (np. drzewa, ściany)
+ HP dla statycznych obiektów (nie musi być zapisywane do DB)
+ chat

+ baza danych dla kont (na start np. synchroniczne SQLite)
+ interest management (dla dynamicznych obiektów)
+ tabela timestampów obiektów statycznych
+ delta update (wg. timestamp) statycznych obiektów
- wiele serwerów fizycznych
  - podział regionów na serwery
- baza danych rozproszona
- AI
- znajdowanie drogi
- preload regionów sąsiednich do aktualnego
- działki graczy do budowania, żeby nie za dużo griefowania było
- instance dungeon

- anti cheat
- rozmowy z NPC

-- fabuła sterowana przez NPC
-- dynamiczne zaciąganie modeli do tego co ma być w danej chwili wyświetlone
-- system questów: generowane przez NPC lub tworzone przez graczy, za równo
    gracze jak i NPC mogą wykonywać questy

## Client Elements

## Game Elements

+ W KOŃCU WYMYŚLIĆ GRE, A NIE DEMO SILNIKA
+ jakieś modele statyczne
+ modele animowane
+ animacje
+ kształty kolizji
+ zapuścić AI do generowania json'ów itemków
+ zrobić karte postaci prawdziwą: hp, mp, str, int, dex, con, ...,
    status effects
+ zrobić ekwipunek postaci/skrzyń/itp. uniwersalny
+ ekwipowanie itemów (zignorować modele postaci np. ze zbrojami)
+ UI karty postaci
+ UI inventory
+ SYSTEM SKILLI
+ system umiejętności
+ loot tables
+ system klas
+ system craftingu
+ system gatheringu surowców

- trading
- auctions

- LOD
- occlusion culling
- batchowanie draw call

