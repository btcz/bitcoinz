### Bootstrap the Blockchain Synchronization

Normally the BitcoinZ client will download the transaction and network information, called the blockchain, from the network by syncing with the other clients. This process can take quite some time as the [BitcoinZ blockchain](https://explorer.btcz.rocks/) is growing bigger and bigger for each day. Luckily there is a safe and fast way to speed up this process. We'll show you how to bootstrap your blockchain to bring your client up to speed in just a few simple steps.

### Requirements

- A fresh install of the BitcoinZ client software.

### Download the blockchain via GitHub

BitcoinZ offers an [github file](https://github.com/btcz/bitcoinz) for bootstrapping purposes that is updated often.

### Importing the blockchain
Exit the BitcoinZ client software if you have it running. Be sure not to have an actively used wallet in use. We are going to copy the download of the blockchain to the BitcoinZ client data directory. You should run the client software at least once so it can generate the data directory. Copy the downloaded bootstrap.dat file into the BitcoinZ data folder.

**For Windows users:**
Open explorer, and type into the address bar:

	%APPDATA%\BitcoinZ

This will open up the data folder. It should look like the image below. Copy over the bootstrap.dat from your download folder to this directory.

**For OSX users:**
Open Finder by pressing Press [shift] + [cmd] + [g] and enter:

	~/Library/Application Support/BitcoinZ/

**For Linux users:**
The directory is hidden in your User folder. Go to:

	~/.bitcoinz/

### Importing the blockchain
Now start the BitcoinZ client software. It should show "Importing blocks from disk" like the image below.

Wait until the import finishes. This take some time (from some hours to a couple of days depending on your hardware) while the BitcoinZ client verifies the imported blockchain.

When the import finishes, the client will download the last missing days not covered by the import. You should see the status changed to "Synchronizing with network...".

Congratulations you have successfully imported the blockchain!

### Is this safe?

Yes, the above method is safe. The download contains only raw blockchain data and the client verifies this on import. Do not download the blockchain from unofficial sources, especially if they provide `*.rev` and `*.sst` files. These files are not verified and can contain malicious edits.
