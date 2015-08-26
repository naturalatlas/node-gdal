if($env:APPVEYOR_REPO_COMMIT_MESSAGE.ToLower().Contains('[publish binary]')) {
    Write-Host '1';
} else {
    Write-Host '0';
}