# SOI zadanie 6 - projekt
Mikołaj Garbowski

Rozszerzona koncepcja implementacji systemu plików.

## Struktury danych

### VirtualDisk
`VirtualDisk` reprezentuje wirtualny dysk z systemem plików, jest zapisywany jako plik binarny

```c++
class VirtualDisk {
private:
    Superblock superblock;
    FreeBlocksBitmap bitmap;
    INode inodes[N_INODES]{};
    Block blocks[N_BLOCKS]{};
    ...  
    
public:
    void saveFile(const std::string &filename, const std::vector<uint8_t> &bytes);
    std::vector<uint8_t> readFile(const std::string &filename);
    void saveToFile(const std::string &path);
    static VirtualDisk *initialize();
    static VirtualDisk *loadFromFile(const std::string &path);
};
```

### Superblock
`Superblock` przechowuje metadane o systemie plików, jest aktualizowany przy odpowiednich operacjach.

```c++
struct Superblock {
    int64_t filesystemCreatedTimestamp;
    uint32_t diskSize;
    uint32_t blockSize;
    uint32_t nFreeBlocks;
    uint8_t nFiles;
    ...
};
```

### FreeBlocksBitmap
`FreeBlocksBitmap` służy do zarządzania wolnymi blokami pamięci.
Po 1 bicie dla bloku danych.

```c++
class FreeBlocksBitmap {
private:
    std::bitset<N_BLOCKS> bitmap;
public:
    ...
    std::vector<uint32_t> allocateFreeBlocks(uint32_t nBlocks);
};
```

### INode
`INode` reprezentuje węzeł indeksowy, wskazuje na zajęte przez plik (obiekt systemu plików) bloki dysku.

```c++
struct INode {
    int64_t createdTimestamp;
    int64_t modifiedTimestamp;
    int64_t accessedTimestamp;
    uint32_t fileSize;
    uint32_t blocks[INODE_BLOCKS];
    uint8_t nLinks;
    uint8_t type;
    bool inUse;
    ...
};
```

### Directory
`Directory` reprezentuje katalog w systemie plików. Zajmuje zawsze dokładnie 1 blok danych.

```c++
struct DirectoryEntry {
    uint8_t iNodeNumber;
    char filename[FILENAME_SIZE]{};
    ...
};

struct Directory {
    DirectoryEntry entries[DIRECTORY_SIZE]{};
    ...
};
```

## Ograniczenia
Wartości stałych są zdefiniowane w pliku `common.h`

```c++
#define BLOCK_SIZE (32 * 1024) // 32kB
#define N_INODES 256
#define N_BLOCKS 4096  // for total disk capacity of 128MB
#define DIRECTORY_SIZE 2048  // BLOCK_SIZE / sizeof(DirectoryEntry)
#define FILENAME_SIZE 15
#define INODE_BLOCKS 16
#define MAX_FILE_SIZE (INODE_BLOCKS * BLOCK_SIZE)
...
```

* Stały rozmiar dysku (tylko części poświęconej na bloki danych) - 128MB
* Maksymalna liczba plików 256 - tyle co i-nodes
* Maksymalny rozmiar pliku 512kB - i-node przechowuje do 16 numerów bloków, każdy po 32kB
* Maksymalna długość nazwy pliku - 15 znaków ASCII
* Maksymalna liczba plików w katalogu 2048 - tyle ile zmieści się w 1 bloku


## Funkcjonalności
Plik `main.cpp` implementuje prostą aplikację konsolową pozwalającą 
* utworzyć wirtualny dysk w wybranym miejscu
* skopiować plik z komputera do katalogu root wirtualnego dysku
* skopiować plik z katalogu root wirtualnego dysku na komputer

Program poprawnie kopiuje plik większy niż 1 blok dysku

Oryginalny obraz:

![Pierwotny obraz](./kot1.jpeg)

Odczytany z wirtualnego dysku:

![Odczytany obraz](./result1.jpeg)

## Koncepcja działania operacji
### Tworzenie wirtualnego dysku
Zaimplementowane w `VirtualDisk *VirtualDisk::initialize()`

### Kopiowanie pliku z dysku systemu na dysk wirtualny
Zaimplementowane w `void saveFile(const std::string &filename, const std::vector<uint8_t> &bytes);`

### Tworzenie katalogu na dysku wirtualnym
* Alokacja bloku danych `VirtualDisk::allocateBlocks(uint32_t nBlocks)`
* Utworzenie i-node z odpowiednimi wartościami `INode::empty()`
* Utworzenie pustego katalogu `Directory::Directory(uint8_t selfINodeNumber, uint8_t parentINodeNumber)`
* Zapisanie katalogu do bloku danych `VirtualDisk::saveDirectory(Directory directory, uint32_t blockIdx)`

### Usunięcie katalogu z dysku wirtualnego
* Sprawdzenie, czy katalog jest pusty (zawiera tylko `.` i `..`)
* Ustawienie flagi `inUse` w i-node na `false`
* Oznaczenie bloku danych w `FreeBlocksBitmap` jako wolny

### Kopiowanie pliku z dysku wirtualnego na dysk systemu
Zaimplementowane w `VirtualDisk::readFile(const std::string &filename)` i aplikacji `main.cpp`

### Wyświetlanie katalogu dysku wirtualnego
* Odnalezienie i-node katalogu zaczynając przeszukiwanie od katalogu root
* Wczytanie bloku danych jako strukturę `Directory`
* Przeiterowanie po pozycjach w katalogu `DirectoryEntry entries[DIRECTORY_SIZE]{};`
* W celu otrzymania informacji o całkowitym rozmiarze katalogu można przeszukiwać rekurencyjnie pliki i podkatalogi
  * sumując rozmiary plików zapisane w `INode` dla rozmiaru faktycznych danych
  * sumując rozmiary zajmowanych bloków: pole `uint32_t blocks[INODE_BLOCKS]` w `INode` dla rozmiaru efektywnie zajętej przestrzeni na dysku

### Tworzenie twardego dowiązania do pliku lub katalogu
* Dodać nową pozycję w istniejącym katalogu (w którym ma powstać dowiązanie)
  * numer i-node istniejącego pliku/katalogu (do którego ma być dowiązanie)
  * nazwa dowiązania
* Inkrementacja pola `uint8_t nLinks` w `INode` pliku, do którego utworzono dowiązanie

### Usuwanie pliku lub dowiązania z wirtualnego dysku
* Odnaleźć i-node usuwanego pliku (dowiązania)
* Zdekrementować licznik dowiązań
  * Koniec operacji, jeśli licznik dalej jest większy od 0
* Usunąć pozycję w katalogu rodzicu
* Ustawić flagę `inUse` w i-node na `false`
* Oznaczyć numery bloków jako wolne w bitmapie

### Dodanie do istniejącego pliku n bajtów
* Zwiększyć wartość `fileSize` w i-node
* Jeśli w ostatnim używanym bloku jest n wolnych bajtów to zakończyć
* Doalokować potrzebną liczbę bloków `VirtualDisk::allocateBlocks(uint32_t nBlocks)`

### Skrócenie istniejącego pliku o n bajtów
* Zmniejszyć wartość `fileSize` w i-node
* Jeśli zmiana przekroczyła granice bloków to zwolnić nieużywane bloki
  * Oznaczyć jako wolne w bitmapie
  * Zaktualizować informację o liczbie wolnych bloków w `Superblock`

### Wyświetlenie informacji o zajętości dysku
* Obliczyć wartość na podstawie liczby wolnych bloków zapisanej w `Superblock` dla efektywnie zajętej przestrzeni
* Obliczyć rekurencyjnie rozmiar katalogu root (jak wyżej) dla przestrzeni zajmowanej przez faktyczne dane