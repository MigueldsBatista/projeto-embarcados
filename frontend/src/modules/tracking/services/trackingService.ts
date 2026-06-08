import client from '@/infrastructure/api/client';
import type {
  DiscoveredTag,
  HeatmapData,
  TrackingBenchmarkState,
  TrackingStatus,
} from '@/shared';

export const fetchDiscovery = async (): Promise<Record<string, DiscoveredTag>> => {
  const response = await client.get<Record<string, DiscoveredTag>>('/tracking/discovery');
  return response.data;
};

export const fetchHeatmap = async (mac: string): Promise<HeatmapData[]> => {
  const response = await client.get(`/tracking/heatmap/${mac}`);
  return response.data;
};

export const fetchLiveTracking = async (): Promise<TrackingStatus[]> => {
  const response = await client.get<TrackingStatus[]>('/tracking/live');
  return response.data;
};

export const fetchBenchmark = async (): Promise<TrackingBenchmarkState> => {
  const response = await client.get<TrackingBenchmarkState>('/tracking/benchmark');
  return response.data;
};
