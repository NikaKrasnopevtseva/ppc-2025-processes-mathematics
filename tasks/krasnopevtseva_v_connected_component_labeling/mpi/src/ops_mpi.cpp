#include "krasnopevtseva_v_connected_component_labeling/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <queue>
#include <unordered_map>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"

namespace {
void MakeNorm(int total, std::vector<int> &parent, int *p_global) {
  auto find_rep = [&parent](int x) -> int {
    int root = x;
    while (root != parent[root]) {
      root = parent[root];
    }
    while (x != root) {
      int next = parent[x];
      parent[x] = root;
      x = next;
    }
    return root;
  };

  std::unordered_map<int, int> root_to_label;
  int next_label = 1;

  for (int i = 0; i < total; ++i) {
    if (p_global[i] != 0) {
      int root = find_rep(p_global[i]);
      if (!root_to_label.contains(root)) {
        root_to_label[root] = next_label++;
      }
    }
  }

  for (int i = 0; i < total; ++i) {
    if (p_global[i] != 0) {
      int root = find_rep(p_global[i]);
      p_global[i] = root_to_label[root];
    }
  }
}

int FindMaxLabel(const int *p_global, int total) {
  int max_label = 0;
  for (int i = 0; i < total; ++i) {
    max_label = std::max(p_global[i], max_label);
  }
  return max_label;
}

void InitializeUnionFind(std::vector<int> &parent, int max_label) {
  parent.resize(max_label + 1);
  for (int i = 0; i <= max_label; ++i) {
    parent[i] = i;
  }
}

int FindRoot(std::vector<int> &parent, int x) {
  if (x < 0 || static_cast<size_t>(x) >= parent.size()) {
    return x;
  }
  while (x != parent[x]) {
    parent[x] = parent[parent[x]];
    x = parent[x];
  }
  return x;
}

void UniteLabels(std::vector<int> &parent, int a, int b) {
  if (a < 0 || static_cast<size_t>(a) >= parent.size() || b < 0 || static_cast<size_t>(b) >= parent.size()) {
    return;
  }
  int ra = FindRoot(parent, a);
  int rb = FindRoot(parent, b);
  if (ra != rb) {
    if (ra < rb) {
      parent[rb] = ra;
    } else {
      parent[ra] = rb;
    }
  }
}

void ProcessRightConnection(int *p_global, int idx, std::vector<int> &parent) {
  if (p_global[idx + 1] != 0 && p_global[idx] != p_global[idx + 1]) {
    UniteLabels(parent, p_global[idx], p_global[idx + 1]);
  }
}

void ProcessBottomConnection(int *p_global, int idx, int n_tmp, std::vector<int> &parent) {
  if (p_global[idx + n_tmp] != 0 && p_global[idx] != p_global[idx + n_tmp]) {
    UniteLabels(parent, p_global[idx], p_global[idx + n_tmp]);
  }
}

void ProcessPixelConnections(int *p_global, int i, int j, int m_tmp, int n_tmp, std::vector<int> &parent) {
  const int idx = (i * n_tmp) + j;
  if (p_global[idx] == 0) {
    return;
  }

  if (j + 1 < n_tmp) {
    ProcessRightConnection(p_global, idx, parent);
  }

  if (i + 1 < m_tmp) {
    ProcessBottomConnection(p_global, idx, n_tmp, parent);
  }
}

void ProcessConnections(int *p_global, int m_tmp, int n_tmp, std::vector<int> &parent) {
  for (int i = 0; i < m_tmp; ++i) {
    for (int j = 0; j < n_tmp; ++j) {
      ProcessPixelConnections(p_global, i, j, m_tmp, n_tmp, parent);
    }
  }
}
}  // namespace

namespace krasnopevtseva_v_connected_component_labeling {

KrasnopevtsevaVCCLMPI::KrasnopevtsevaVCCLMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrasnopevtsevaVCCLMPI::ValidationImpl() {
  const auto &[height, width, data] = GetInput();

  if (data.empty()) {
    return false;
  }
  if (height <= 0 || width <= 0) {
    return false;
  }
  const size_t total_size = static_cast<size_t>(height) * static_cast<size_t>(width);
  if (total_size != data.size()) {
    return false;
  }

  return std::ranges::all_of(data, [](int pixel) { return pixel == 0 || pixel == 1; });
}

bool KrasnopevtsevaVCCLMPI::PreProcessingImpl() {
  const auto &[height, width, data] = GetInput();
  GetOutput() = std::vector<int>(data.size(), 0);
  return true;
}

bool KrasnopevtsevaVCCLMPI::RunImpl() {
  const auto &[height, width, data] = GetInput();
  int m_tmp = height;
  int n_tmp = width;

  int proc_rank = 0;
  int proc_count = 0;
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

  MPI_Scatterv(data.data(), counts.data(), displacements.data(), MPI_INT, local_image.data(), local_pixel_count,
               MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_labels(local_pixel_count, 0);

  if (local_pixel_count > 0) {
    int local_rows = local_pixel_count / n_tmp;
    int start_label = ((proc_rank + 1) * 1000000) + 1;

    MPIBfs(local_image.data(), local_pixel_count, local_labels.data(), start_label, local_rows);
  }

  std::vector<int> global_labels;
  if (proc_rank == 0) {
    global_labels.resize(static_cast<size_t>(m_tmp) * static_cast<size_t>(n_tmp), 0);
  }

  int *sendbuf = (local_pixel_count > 0) ? local_labels.data() : nullptr;
  int sendcount = (local_pixel_count > 0) ? local_pixel_count : 0;

  MPI_Gatherv(sendbuf, sendcount, MPI_INT, global_labels.data(), counts.data(), displacements.data(), MPI_INT, 0,
              MPI_COMM_WORLD);

  std::vector<int> final_result;
  if (proc_rank == 0) {
    final_result = MakeMPIResult(global_labels, m_tmp, n_tmp);
  }

  int result_size = 0;
  if (proc_rank == 0) {
    result_size = static_cast<int>(final_result.size());
  }

  MPI_Bcast(&result_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (proc_rank != 0) {
    final_result.resize(result_size);
  }

  MPI_Bcast(final_result.data(), result_size, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = final_result;
  MPI_Barrier(MPI_COMM_WORLD);

  return true;
}

bool KrasnopevtsevaVCCLMPI::PostProcessingImpl() {
  return true;
}

bool KrasnopevtsevaVCCLMPI::IsValidMPI(int nr, int nc, int local_rows, int n_tmp) {
  return nr >= 0 && nr < local_rows && nc >= 0 && nc < n_tmp;
}

void KrasnopevtsevaVCCLMPI::BFSCheck(const int *p_local_image, int curr_label, int *p_local_labels, int local_rows,
                                     Point cp, std::queue<Point> &bfs_queue, int n_tmp) {
  const std::vector<Point> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

  for (const auto &dir : directions) {
    int nr = cp.x + dir.x;
    int nc = cp.y + dir.y;
    if (IsValidMPI(nr, nc, local_rows, n_tmp)) {
      int idx = (nr * n_tmp) + nc;
      if (idx >= 0 && idx < local_rows * n_tmp) {
        if (p_local_image[idx] == 1 && p_local_labels[idx] == 0) {
          p_local_labels[idx] = curr_label;
          bfs_queue.push({nr, nc});
        }
      }
    }
  }
}

void KrasnopevtsevaVCCLMPI::MPIBfs(int *p_local_image, int local_pixel_count, int *p_local_labels, int curr_label,
                                   int local_rows) {
  const auto &[height, width, data] = GetInput();
  int n_tmp = width;

  for (int idx = 0; idx < local_pixel_count; ++idx) {
    if (p_local_image[idx] == 1 && p_local_labels[idx] == 0) {
      std::queue<Point> bfs_queue;
      int local_row = idx / n_tmp;
      int col = idx % n_tmp;

      bfs_queue.push({local_row, col});
      p_local_labels[idx] = curr_label;

      while (!bfs_queue.empty()) {
        Point current = bfs_queue.front();
        bfs_queue.pop();

        BFSCheck(p_local_image, curr_label, p_local_labels, local_rows, current, bfs_queue, n_tmp);
      }

      curr_label++;
    }
  }
}

std::vector<int> KrasnopevtsevaVCCLMPI::MakeMPIResult(const std::vector<int> &global_labels, int m_tmp, int n_tmp) {
  const int total = m_tmp * n_tmp;
  std::vector<int> result = global_labels;
  int *p_global = result.data();

  const int max_label = FindMaxLabel(p_global, total);

  if ((max_label == 0) || (max_label > 10000000)) {
    return result;
  }

  std::vector<int> parent;
  InitializeUnionFind(parent, max_label);
  ProcessConnections(p_global, m_tmp, n_tmp, parent);
  MakeNorm(total, parent, p_global);

  return result;
}
}  // namespace krasnopevtseva_v_connected_component_labeling
