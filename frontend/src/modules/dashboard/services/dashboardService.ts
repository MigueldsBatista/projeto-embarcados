import client from '@/infrastructure/api/client';
import type { AccessLog, Summary } from '@/shared';

export const fetchLogs = async (): Promise<AccessLog[]> => {
  const response = await client.get('/metrics/logs');
  return response.data;
};

export const fetchSummary = async (): Promise<Summary> => {
  const response = await client.get('/metrics/summary');
  return response.data;
};
