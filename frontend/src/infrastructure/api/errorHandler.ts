import axios from 'axios';

export function handleApiError(error: unknown) {
  if (axios.isAxiosError(error)) {
    alert(error.response?.data?.detail || 'An error occurred');
  } else {
    alert('An unexpected error occurred');
  }
}
