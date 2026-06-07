import client from '@/infrastructure/api/client';
import type { Card } from '@/shared';

export const fetchCards = async (): Promise<Card[]> => {
  const response = await client.get('/cards/');
  return response.data;
};

export const createCard = async (newCard: Partial<Card>): Promise<Card> => {
  const response = await client.post('/cards/', newCard);
  return response.data;
};

export const updateCard = async (uid: string, data: Partial<Card>): Promise<Card> => {
  const response = await client.put(`/cards/${uid}`, data);
  return response.data;
};

export const deleteCard = async (uid: string): Promise<void> => {
  const response = await client.delete(`/cards/${uid}`);
  return response.data;
};
