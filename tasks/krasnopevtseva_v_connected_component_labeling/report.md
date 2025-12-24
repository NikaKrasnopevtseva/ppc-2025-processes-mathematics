# Маркировка компонент на бинарном изображении

- Студент: <Краснопевцева Вероника Дмитриевна>, группа 3823Б1ПМоп3
- Технологии: SEQ | MPI 
- Вариант: 31

## 1. Введение

Задача маркировки компонент связности на бинарном изображении является классической задачей компьютерного зрения и обработки изображений. Алгоритм находит и помечает связные области пикселей на бинарном изображении (0 - фон, 1 - объект). Эта задача имеет широкое применение в медицинской визуализации, распознавании объектов, анализе документов и других областях. Особую сложность представляет обработка больших изображений, где требуется высокая производительность.

## 2. Постановка задачи
### Задача: реализация последовательной (SEQ) и параллельной (MPI) версий метода маркировки компонент на бинарном изображении.


### Формат входных данных:
- Высота изображения (int)
- Ширина изображения (int)
- Вектор пикселей изображения размером height × width (std::vector<int>), где 0 - фон, 1 - объект 

### Формат выходных данных:
- Вектор меток того же размера (std::vector<int>), где 0 - фон, а 1, 2, 3, ... - метки связных компонент

## 3. Описание алгоритма

Для последовательной версии использован алгоритм на основе BFS (поиска в ширину):

1. Инициализация массива меток нулями

2. Проход по всем пикселям изображения

3. Если пиксель имеет значение 1 и не помечен:

4. Запуск BFS для маркировки всей связной компоненты

5. Присвоение новой уникальной метки

6. Продолжение до обработки всех пикселей

## 4. Схема распараллеливания

Для параллельной версии использована гибридная схема:

- Распределение данных: изображение делится по строкам между процессами

- Локальная маркировка: каждый процесс маркирует свою часть BFS с уникальными метками

- Сбор результатов: метки собираются на корневом процессе

- Объединение меток: использование DSU (системы непересекающихся множеств) для объединения меток на границах

- Нормализация: перенумерация меток в последовательность 1, 2, 3, ...

- Распространение: результат рассылается всем процессам

### 4.1 Распределение данных
Изображение делится по строкам между процессами:  
Каждый процесс получает примерно равное количество строк  
### 4.2 Уникальные метки
Для избежания конфликтов меток между процессами:  
Каждый процесс начинает маркировку с уникальной базовой метки: (proc_rank + 1) × 1000000 + 1  
Это гарантирует отсутствие пересечений между процессами
### 4.3 Объединение меток
После сбора всех локальных меток:  
Построение DSU для всех меток    
Объединение меток соседних пикселей по границам  
Перенумерация меток в последовательную нумерацию

- RunImpl() - основной метод выполнения алгоритма
- MakeMPIResult() - объединение и нормализация меток
- MPIBfs() - локальная маркировка BFS
- MakeNorm() - нормализация меток через DSU
## 5. Детали реализации 

Ключевые классы и функции: 
- класс KrasnopevtsevaVCCLMPI с реализацией параллельной версии метода
- класс KrasnopevtsevaVCCLSEQ с реализацией последовательной версии метода
- определение пространства имен namespace krasnopevtseva_v_connected_component_labeling с определеним используемых типов данных:  
        using InType = std::tuple<int, int, std::vector<int>>;    
        using OutType = std::vector<int>;   
        using TestType = std::tuple<InType, std::string>;   
        using BaseTask = ppc::task::Task<InType, OutType>;   

## 6. Окружение
- Windows: AMD Ryzen 7 5700X 8-Core Processor, 32.0 ГБ ОП, Windows 11 Pro 25H2
- Набор инструментов: DevContainer:compiler gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0, Open MPI 4.1.6, build type Release

## 7. Результаты

### 7.1 Корректность


### 7.2 Производительность

Тесты производительности используют изображения 5000×5000 пикселей с детерминированными паттернами:
- Центральный квадрат 4000×4000
- Горизонтальные и вертикальные линии
- Отдельные маленькие компоненты

| Mode        | Count | Time,ms | Speedup | Efficiency |
|-------------|-------|---------|---------|------------|
| seq         | 1     | 819     | 1.00    | N/A        |
| mpi         | 2     | 3286    | 0.249   | 12.5%      |
| mpi         | 4     | 3454    | 0.237   | 5.9%       |

Расчеты:  
- Speedup = T_seq / T_parallel  
- Для 2 процессов: 0.819 / 3.286 = 0.249
- Для 4 процессов: 0.819 / 3.454 = 0.237

- Efficiency = Speedup / Count × 100%
- Для 2 процессов: 0.249 / 2 × 100% = 12.5%
- Для 4 процессов: 0.237 / 4 × 100% = 5.9%

## 8. Заключение

Реализованы корректные последовательная и параллельная версии алгоритма маркировки компонент связности. Текущая MPI реализация медленнее последовательной - основная проблема это накладные расходы на коммуникацию между процессами.

## 9. Приложения
```cpp
bool KrasnopevtsevaVCCLMPI::RunImpl() {
  const auto &[height, width, data] = GetInput();
  int m_tmp = height;
  int n_tmp = width;
  
  int proc_rank, proc_count;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
  
  MPI_Bcast(&m_tmp, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n_tmp, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  std::vector<int> counts(proc_count, 0);
  std::vector<int> displacements(proc_count, 0);
  
  int base_rows = m_tmp / proc_count;
  int extra_rows = m_tmp % proc_count;
  int current_row = 0;
  
  for (int proc = 0; proc < proc_count; ++proc) {
    int proc_rows = base_rows + (proc < extra_rows ? 1 : 0);
    counts[proc] = proc_rows * n_tmp;
    displacements[proc] = current_row * n_tmp;
    current_row += proc_rows;
  }
  
  int local_pixel_count = counts[proc_rank];
  std::vector<int> local_image(local_pixel_count);
  std::vector<int> local_labels(local_pixel_count, 0);
  
  MPI_Scatterv(data.data(), counts.data(), displacements.data(), MPI_INT,
               local_image.data(), local_pixel_count, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (local_pixel_count > 0) {
    int local_rows = local_pixel_count / n_tmp;
    int start_label = (proc_rank + 1) * 1000000 + 1;
    MPIBfs(local_image.data(), local_pixel_count, local_labels.data(), 
           start_label, local_rows, displacements[proc_rank] / n_tmp);
  }
  
  std::vector<int> global_labels;
  if (proc_rank == 0) {
    global_labels.resize(m_tmp * n_tmp, 0);
  }
  
  int* sendbuf = (local_pixel_count > 0) ? local_labels.data() : nullptr;
  int sendcount = (local_pixel_count > 0) ? local_pixel_count : 0;
  
  MPI_Gatherv(sendbuf, sendcount, MPI_INT,
              global_labels.data(), counts.data(), displacements.data(), MPI_INT,
              0, MPI_COMM_WORLD);
  
  std::vector<int> final_result;
  if (proc_rank == 0) {
    final_result = MakeMPIResult(global_labels, m_tmp, n_tmp);
  }
  
  int result_size = 0;
  if (proc_rank == 0) result_size = final_result.size();
  MPI_Bcast(&result_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (proc_rank != 0) final_result.resize(result_size);
  MPI_Bcast(final_result.data(), result_size, MPI_INT, 0, MPI_COMM_WORLD);
  
  GetOutput() = final_result;
  return true;
}
```