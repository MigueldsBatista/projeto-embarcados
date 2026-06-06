export interface Card {
  id: number;
  uid: string;
  name: string;
  role: string;
  is_active: boolean;
  tracker_mac?: string;
  created_at: string;
}

export interface TrackingStatus {
  student_name: string;
  mac: string;
  status: 'online' | 'offline';
  rssi: number | null;
  zone: 'Perto' | 'Medio' | 'Longe' | 'Desconhecida';
  scanner: string | null;
  last_seen: string | null;
}

export interface DiscoveredTag {
  rssi: number;
  scanner?: string;
  timestamp: string;
}

export interface TrackingBenchmarkSummary {
  approach: 'inefficient' | 'circular';
  n_target: number;
  processed: number;
  latency_min_us: number;
  latency_max_us: number;
  latency_avg_us: number;
  heap_min: number;
  heap_max: number;
  dropped: number;
  updated_at: string;
  summary_key: string;
}

export interface TrackingBenchmarkPerfPoint {
  approach: 'inefficient' | 'circular';
  n_target: number;
  index: number;
  latency_us: number;
  heap_free: number;
  received_at: string;
}

export interface TrackingBenchmarkBatchSample {
  mac: string;
  rssi: number;
  scanner: string;
  t_us: number;
}

export interface TrackingBenchmarkBatch {
  approach: 'inefficient' | 'circular';
  n_target: number;
  batch: number;
  samples: TrackingBenchmarkBatchSample[];
  received_at: string;
}

export interface TrackingBenchmarkState {
  summaries: TrackingBenchmarkSummary[];
  perf_points: TrackingBenchmarkPerfPoint[];
  recent_batches: TrackingBenchmarkBatch[];
}

export interface AccessLog {
  id: number;
  uid: string;
  status: 'authorized' | 'unknown' | 'inactive';
  timestamp: string;
}

export interface Summary {
  total_scans: number;
  unknown_scans: number;
}
