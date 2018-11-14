
## Data structures

NOTE: each anchor has an ID [0, 1, ...]

#### `received_data` 

```
	[ MAC, timestamp, n_rssi_received, [ rssi ], packet_signature ]
```

 - n_rssi_received is the length of the list with the RSSIs received from anchors
 - packet signature is related to the MAC, and is useful to attack mac randomization; is composed by different fields in the probe request
 - [ rssi ] is a list with the measurements received from all anchors, in position i will be the RSSI received from anchor i
 
Overall:
 - `received_data` is a map containing for each MAC, for each timestamp interval (1 second) the measurements from all the anchors.
 - shared among all Receivers:
 
##### when a Receiver receives a packet:

```C
	id = anchor_id;
	if ( !received_data.contains(MAC) ) {
	
		received_data.push(MAX, normalized_timestamp, 1, rssi[id] = RSSI, packet_signature/blob);
	
	} else {
		
		entry = received_data.get(MAC, nomalized_timestamp);
		entry.n_rssi_received++;
		entry.rssi[id] = RSSI;
		
		if ( entry.n_rssi_received == N_ANCHORS ) {
			localization_buffer.push(entry);
			localization_thread.notify();
			received_data.delete(MAC, timestamp);
			}
	}

```

#### Localization thread (and localization_buffer)

 - the localization thread wakes up when has data to process and goes to sleep when `localizatin_buffer` is empty
 
##### when localization thread is awaken:

```C

	while ( 1 ) {
	
		while ( !localization_buffer.is_empty() ) {
			entry = localization_buffer.pop();
			point = triangolarization(entry.rssi, anchors_positions);
			db.devices.add(entry.MAC, entry.timestamp, point.x, point.y);
			
			if ( first_time_seen(entry.MAC) )
				db.signatures.add(entry.MAC, entry.packet_signature);
		}
		
		sleep();
	}

```

#### Database

Only 2 tables may be enough for everything.
```SQL
	signatures(MAC, packet_signature);
	devices(MAC, timestamp, X, Y);
```
























**Edit a file, create a new file, and clone from Bitbucket in under 2 minutes**

When you're done, you can delete the content in this README and update the file with details for others getting started with your repository.

*We recommend that you open this README in another tab as you perform the tasks below. You can [watch our video](https://youtu.be/0ocf7u76WSo) for a full demo of all the steps in this tutorial. Open the video in a new tab to avoid leaving Bitbucket.*

---

## Edit a file

You’ll start by editing this README file to learn how to edit a file in Bitbucket.

1. Click **Source** on the left side.
2. Click the README.md link from the list of files.
3. Click the **Edit** button.
4. Delete the following text: *Delete this line to make a change to the README from Bitbucket.*
5. After making your change, click **Commit** and then **Commit** again in the dialog. The commit page will open and you’ll see the change you just made.
6. Go back to the **Source** page.

---

## Create a file

Next, you’ll add a new file to this repository.

1. Click the **New file** button at the top of the **Source** page.
2. Give the file a filename of **contributors.txt**.
3. Enter your name in the empty file space.
4. Click **Commit** and then **Commit** again in the dialog.
5. Go back to the **Source** page.

Before you move on, go ahead and explore the repository. You've already seen the **Source** page, but check out the **Commits**, **Branches**, and **Settings** pages.

---

## Clone a repository

Use these steps to clone from SourceTree, our client for using the repository command-line free. Cloning allows you to work on your files locally. If you don't yet have SourceTree, [download and install first](https://www.sourcetreeapp.com/). If you prefer to clone from the command line, see [Clone a repository](https://confluence.atlassian.com/x/4whODQ).

1. You’ll see the clone button under the **Source** heading. Click that button.
2. Now click **Check out in SourceTree**. You may need to create a SourceTree account or log in.
3. When you see the **Clone New** dialog in SourceTree, update the destination path and name if you’d like to and then click **Clone**.
4. Open the directory you just created to see your repository’s files.

Now that you're more familiar with your Bitbucket repository, go ahead and add a new file locally. You can [push your change back to Bitbucket with SourceTree](https://confluence.atlassian.com/x/iqyBMg), or you can [add, commit,](https://confluence.atlassian.com/x/8QhODQ) and [push from the command line](https://confluence.atlassian.com/x/NQ0zDQ).