# Verifica se está rodando como Administrador
if (!([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "ERRO: Execute este script como ADMINISTRADOR!" -ForegroundColor Red
    Pause
    exit
}

Write-Host "--- Gerenciador de Dispositivos WSL ---" -ForegroundColor Cyan

# Lista os dispositivos
usbipd list

$busid = Read-Host "`nDigite o BUSID do dispositivo que deseja conectar ao WSL (ex: 2-3)"

if ($busid -ne "") {
    Write-Host "Tentando vincular (bind) o dispositivo $busid..." -ForegroundColor Yellow
    usbipd bind --busid $busid
    
    Write-Host "Anexando (attach) ao WSL..." -ForegroundColor Yellow
    usbipd attach --wsl --busid $busid
    
    Write-Host "`nConcluído! Verifique no WSL usando 'lsusb'." -ForegroundColor Green
} else {
    Write-Host "Nenhum BUSID inserido. Operação cancelada." -ForegroundColor Red
}

Pause