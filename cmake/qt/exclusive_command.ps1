

# param(
#     [Parameter(
#         Mandatory=$True,
#         Position = 0
#     )]
#     [string]
#     $command,
    
#     [Parameter(
#         Mandatory=$False,
#         ValueFromRemainingArguments=$true,
#         Position = 1
#     )][string[]]
#     $arguments
# )

$command   = $args[0]
$arguments = $args[1..($args.Count - 1)]

# Define a unique name for the mutex to be used across multiple script executions
$mutexName = "Global\EcalExclusiveCommandMutex"

# Function to acquire the lock (mutex)
function Acquire-Lock {
    $global:mutex = New-Object System.Threading.Mutex($false, $mutexName)
    Write-Host "[eCAL exclusive command]: Attempting to acquire the lock..."
    
    try {
        # Wait for the mutex to be acquired (this is a blocking call until the lock is acquired)
        $global:mutex.WaitOne()
        Write-Host "[eCAL exclusive command]: Lock acquired."
    } catch {
        Write-Error "[eCAL exclusive command]: Failed to acquire the lock: $_"
        exit 1
    }
}

# Function to release the lock
function Release-Lock {
    Write-Host "[eCAL exclusive command] Releasing lock..."
    $global:mutex.ReleaseMutex()
    $global:mutex.Dispose()
}

# Main execution flow
try {
    # Acquire the lock before running the command
    Acquire-Lock

    # Execute the command with the list of arguments
    Write-Host "[eCAL exclusive command] Executing command: `"$command`" $($arguments -join ' ')"
    Start-Process $command -ArgumentList $arguments -Wait -NoNewWindow

} catch {
    Write-Error "[eCAL exclusive command] An error occurred: $_"
} finally {
    # Ensure that the lock is released after execution
    Release-Lock
}