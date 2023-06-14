$Files = Get-ChildItem -Path "${PSScriptRoot}/../GreatCity/Source" -File -Recurse -Include *.c,*.cpp,*.h -Exclude ThirdParty*

foreach ($File in $Files) {
	if(!$File.FullName.Contains("ThirdParty")) {
		Write-Output "Formatting ${File}"
		& clang-format.exe @("-i", "-style=file", $File.FullName)
	}
}
