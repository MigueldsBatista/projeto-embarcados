import type { MaybeRefOrGetter } from 'vue';

export const TRACKING_QUERY_KEYS = {
  DISCOVERY: ['tracking', 'discovery'] as const,
  LIVE: ['tracking', 'live'] as const,
  HEATMAP: (mac: MaybeRefOrGetter<string | null>) => ['tracking', 'heatmap', mac] as const,
};
