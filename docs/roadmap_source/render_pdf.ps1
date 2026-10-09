param([string]$PdfPath,[string]$OutputDir,[int]$Width=850)
Add-Type -AssemblyName System.Runtime.WindowsRuntime
[Windows.Data.Pdf.PdfDocument, Windows.Data.Pdf, ContentType=WindowsRuntime] | Out-Null
[Windows.Data.Pdf.PdfPageRenderOptions, Windows.Data.Pdf, ContentType=WindowsRuntime] | Out-Null
[Windows.Storage.StorageFile, Windows.Storage, ContentType=WindowsRuntime] | Out-Null
[Windows.Storage.StorageFolder, Windows.Storage, ContentType=WindowsRuntime] | Out-Null
[Windows.Storage.Streams.IRandomAccessStream, Windows.Storage.Streams, ContentType=WindowsRuntime] | Out-Null
$taskMethod = [System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object { $_.Name -eq 'AsTask' -and $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1' } | Select-Object -First 1
$actionMethod = [System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object { $_.Name -eq 'AsTask' -and -not $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncAction' } | Select-Object -First 1
function AwaitResult($Operation, $ResultType) { $task = $taskMethod.MakeGenericMethod($ResultType).Invoke($null,@($Operation)); $task.Wait(); return $task.Result }
function AwaitAction($Operation) { $task = $actionMethod.Invoke($null,@($Operation)); $task.Wait() }
$file = AwaitResult ([Windows.Storage.StorageFile]::GetFileFromPathAsync($PdfPath)) ([Windows.Storage.StorageFile])
$pdf = AwaitResult ([Windows.Data.Pdf.PdfDocument]::LoadFromFileAsync($file)) ([Windows.Data.Pdf.PdfDocument])
$folder = AwaitResult ([Windows.Storage.StorageFolder]::GetFolderFromPathAsync($OutputDir)) ([Windows.Storage.StorageFolder])
for ($i=0; $i -lt $pdf.PageCount; $i++) {
  $page = $pdf.GetPage($i)
  $output = AwaitResult ($folder.CreateFileAsync(('page-{0:D2}.png' -f ($i+1)), [Windows.Storage.CreationCollisionOption]::ReplaceExisting)) ([Windows.Storage.StorageFile])
  $stream = AwaitResult ($output.OpenAsync([Windows.Storage.FileAccessMode]::ReadWrite)) ([Windows.Storage.Streams.IRandomAccessStream])
  $options = New-Object Windows.Data.Pdf.PdfPageRenderOptions
  $options.DestinationWidth = $Width
  AwaitAction ($page.RenderToStreamAsync($stream, $options))
  $stream.Dispose(); $page.Dispose()
}
Write-Output ('Rendered {0} pages' -f $pdf.PageCount)
