	const int DESC_SIZE = PATCH_SIZE * 3;
	const int GRID_UCELLS = 3;
	const int GRID_VCELLS = 3;
	const int GRID_MARGIN = (DESC_SIZE - 1) / 2;
	const int GRID_SEPAR = PATCH_SIZE;

	const double PERT_VLIN = 4.0; // m/s per sqrt(s)
	const double PERT_VANG = 1.0; // rad/s per sqrt(s)



//======= 1. Match

					// the list of observations sorted by information gain
					map<double, observation_ptr_t> obsSortedList;


					// loop all observations
					for (SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter
					    != senPtr->observationList().end(); obsIter++) {
						obsPtr = *obsIter;

						obsPtr->clearEvents();
						obsPtr->measurement.matchScore = 0;


						// 1a. project
						obsPtr->project();


						// 1b. check visibility
						obsPtr->predictVisibility();

						if (obsPtr->isVisible()) {


							// Add to tesselation grid for active search
							asGrid.addPixel(obsPtr->expectation.x());


							// predict information gain
							obsPtr->predictInfoGain();


							// add to sorted list
							obsSortedList[obsPtr->expectation.infoGain] = obsPtr;
						} // visible obs
					} // for each obs


					// loop only the N_UPDATES most interesting obs, from largest info gain to smallest
					for (map<double, observation_ptr_t>::reverse_iterator obsIter = obsSortedList.rbegin(); obsIter
					    != obsSortedList.rend(); obsIter++) {
						obsPtr = obsIter->second;


						// 1a. project
						obsPtr->project();


						// 1b. check visibility
						obsPtr->predictVisibility();

						if (obsPtr->isVisible()) {


							//cout << "info gain " << obsPtr->expectation.infoGain << endl;

							if (numObs < N_UPDATES) {


								// update counter
								obsPtr->counters.nSearch++;


								// 1c. predict appearance
								obsPtr->predictAppearance();


								//						senPtr->dataManager.match(rawPtr, obsPtr);

								// 1d. search appearence in raw
								int xmin, xmax, ymin, ymax;
								double dx, dy;
								dx = 3.0 * sqrt(obsPtr->expectation.P(0, 0) + 1.0);
								dy = 3.0 * sqrt(obsPtr->expectation.P(1, 1) + 1.0);
								xmin = (int) (obsPtr->expectation.x(0) - dx);
								xmax = (int) (obsPtr->expectation.x(0) + dx + 0.9999);
								ymin = (int) (obsPtr->expectation.x(1) - dy);
								ymax = (int) (obsPtr->expectation.x(1) + dy + 0.9999);

								cv::Rect roi(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

								kernel::Chrono match_chrono;
								senPtr->getRaw()->match(RawAbstract::ZNCC, obsPtr->predictedAppearance, roi, obsPtr->measurement,
								                        obsPtr->observedAppearance);
								total_match_time += match_chrono.elapsedMicrosecond();


								/*
								 // DEBUG: save some appearances to file
								 ((AppearanceImagePoint*)(((DescriptorImagePoint*)(obsPtr->landmark().descriptorPtr.get()))->featImgPntPtr->appearancePtr.get()))->patch.save("descriptor_app.png");
								 ((AppearanceImagePoint*)(obsPtr->predictedAppearance.get()))->patch.save("predicted_app.png");
								 ((AppearanceImagePoint*)(obsPtr->observedAppearance.get()))->patch.save("matched_app.png");
								 */
								// DEBUG: display predicted appearances on image, disable it when operating normally because can have side effects
								if (SHOW_PATCH) {
									AppearanceImagePoint * appImgPtr =
									    PTR_CAST<AppearanceImagePoint*> (obsPtr->predictedAppearance.get());
									jblas::veci shift(2);
									shift(0) = (appImgPtr->patch.width() - 1) / 2;
									shift(1) = (appImgPtr->patch.height() - 1) / 2;
									appImgPtr->patch.robustCopy(*PTR_CAST<RawImage*> (senPtr->getRaw().get())->img, 0, 0,
									                            obsPtr->expectation.x(0) - shift(0), obsPtr->expectation.x(1) - shift(1));
								}


								// 1e. if feature is found
								if (obsPtr->getMatchScore() > 0.90) {
									obsPtr->counters.nMatch++;
									obsPtr->events.matched = true;
									obsPtr->computeInnovation();


									// 1f. if feature is inlier
									if (obsPtr->compatibilityTest(3.0)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
										numObs++;
										obsPtr->counters.nInlier++;
										kernel::Chrono update_chrono;
										obsPtr->update();
										total_update_time += update_chrono.elapsedMicrosecond();
										obsPtr->events.updated = true;
									} // obsPtr->compatibilityTest(3.0)

								} // obsPtr->getScoreMatchInPercent()>80

							} // number of observations

						} // obsPtr->isVisible()

						// cout << "\n-------------------------------------------------- " << endl;
						// cout << *obsPtr << endl;

					} // foreach observation

				// 2. init new landmarks

					if (mapPtr->unusedStates(LandmarkAnchoredHomogeneousPoint::size())) {

						ROI roi;
						if (asGrid.getROI(roi)) {

							feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(PATCH_SIZE * 3, PATCH_SIZE * 3, CV_8U));
							if (senPtr->getRaw()->detect(RawAbstract::HARRIS, featPtr, &roi)) {
								//							cout << "\n-------------------------------------------------- " << endl;
								//							cout << "Detected pixel: " << featPtr->measurement.x() << endl;

								//((AppearanceImagePoint*)(featPtr->appearancePtr.get()))->patch.save("detected_patch.png");

								// 2a. create lmk object
								ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr)); // add featImgPnt in constructor
								lmkPtr->setId();
								lmkPtr->linkToParentMap(mapPtr);


								// 2b. create obs object
								observation_ptr_t obsPtr = obsFact.create(senPtr, lmkPtr);
								obsPtr->setId();


								// 2c. fill data for this obs
								obsPtr->events.visible = true;
								obsPtr->events.measured = true;
								obsPtr->measurement.x(featPtr->measurement.x());


								// 2d. compute and fill stochastic data for the landmark
								obsPtr->backProject();


								// 2e. Create lmk descriptor
								vec7 globalSensorPose = senPtr->globalPose();
								desc_img_pnt_ptr_t descPtr(new DescriptorImagePoint(featPtr, globalSensorPose, obsPtr));
								lmkPtr->setDescriptor(descPtr);


								// Complete SLAM graph with all other obs
								// mapPtr->completeObservationsInGraph(senPtr, lmkPtr); // FIXME

								//							cout << "\n-------------------------------------------------- " << endl;
								//							cout << *lmkPtr << endl;
								//							cout << "Initialized lmk: " << lmkPtr->id() << endl;

							} // detect()
						} // getROI()
					} // unusedStates()
>>				>>>>> master

				had_data = true;



// 3. MM::Reparam+delete

			// Landmark reparametrization and deletion management
			cout << " ; ";
			for (MapAbstract::LandmarkList::iterator lmkIter =
					mapPtr->landmarkList().begin(); lmkIter != mapPtr->landmarkList().end(); lmkIter++) {
				landmark_ptr_t lmkPtr = *lmkIter;
				if (lmkPtr->needToDie()) {
					cout << "-" << lmkPtr->id() << " ";
					lmkIter++; // move iterator before killing list member
					lmkPtr->suicide();
					lmkIter--;
				}
				else {
					if (lmkPtr->needToReparametrize()) {
						cout << "R" << lmkPtr->id() << " ";
						//						lmkIter++; // move iterator before killing list member
						//						lmkPtr->reparametrize();
						//						lmkIter--; // restore iterator after killing list member
					}
				}
			}
			cout << endl;


